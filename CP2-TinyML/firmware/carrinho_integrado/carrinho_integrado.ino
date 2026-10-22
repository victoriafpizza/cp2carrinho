#include <Arduino.h>
#include <Bluepad32.h>
#include <CarrinhoLuz_inferencing.h> // biblioteca real exportada do Edge Impulse
#include <strings.h>

// Hardware informado pelo grupo
static constexpr int pinoSensor=39;
static constexpr int PinoPWM_Esquerdo=19, pinoIN_E1=18, pinoIN_E2=5;
static constexpr int PinoPWM_Direito=15, pinoIN_D1=4, pinoIN_D2=2;
static constexpr int freq=5000, resolucao=8;
static constexpr int canalPWM_E=0, canalPWM_D=1;
static constexpr int velocidadeCarrinho=200;
static constexpr float FATOR_REDUZIR=0.60f, CONFIANCA_MINIMA=0.70f;
static constexpr uint32_t TIMEOUT_CONTROLE_MS=400;
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static volatile bool iaReduzir=true; // seguro antes de primeira classificação
static volatile int baseEsq=0,baseDir=0;
static uint32_t ultimoComandoMs=0;
ControllerPtr myControllers[BP32_MAX_CONTROLLERS] = {};

float lerSensorEscalaIA() {
  int bruto=analogRead(pinoSensor);
  return float(constrain(map(bruto,0,4095,0,1023),0,1023));
}
void onConnectedController(ControllerPtr ctl) {
  for(int i=0;i<BP32_MAX_CONTROLLERS;i++) if(!myControllers[i]) {
    myControllers[i]=ctl;Serial.printf("Controle conectado: %d\n",i);break;
  }
}
void onDisconnectedController(ControllerPtr ctl) {
  for(int i=0;i<BP32_MAX_CONTROLLERS;i++) if(myControllers[i]==ctl) {
    myControllers[i]=nullptr;Serial.printf("Controle desconectado: %d\n",i);break;
  }
  baseEsq=baseDir=0;
}
void pwmEsq(int v) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(PinoPWM_Esquerdo,abs(v));
#else
  ledcWrite(canalPWM_E,abs(v));
#endif
}
void pwmDir(int v) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(PinoPWM_Direito,abs(v));
#else
  ledcWrite(canalPWM_D,abs(v));
#endif
}
void moverEsquerdo(int v) {
  v=constrain(v,-255,255);
  digitalWrite(pinoIN_E1,v>0);digitalWrite(pinoIN_E2,v<0);pwmEsq(v);
}
void moverDireito(int v) {
  v=constrain(v,-255,255);
  digitalWrite(pinoIN_D1,v>0);digitalWrite(pinoIN_D2,v<0);pwmDir(v);
}
void aplicarMotores() {
  int e=baseEsq,d=baseDir;
  if(!ultimoComandoMs || millis()-ultimoComandoMs>TIMEOUT_CONTROLE_MS) e=d=0;
  float fator=iaReduzir?FATOR_REDUZIR:1.0f;
  moverEsquerdo(int(e*fator));moverDireito(int(d*fator));
}
void processControllers() {
  for(auto ctl:myControllers) if(ctl && ctl->isConnected()) {
    if(ctl->hasData()) {
      uint16_t buttons=ctl->buttons();int e=0,d=0;
      if(buttons&0x0008) e=d=velocidadeCarrinho; // Y: frente
      else if(buttons&0x0001) e=d=-velocidadeCarrinho; // A: trás
      else if(buttons&0x0004) {e=-velocidadeCarrinho;d=velocidadeCarrinho;} // X esquerda
      else if(buttons&0x0002) {e=velocidadeCarrinho;d=-velocidadeCarrinho;} // B direita
      baseEsq=e;baseDir=d;ultimoComandoMs=millis();
    }
  }
}
static void aguardarUs(uint32_t us) {
  if(us>=10000) vTaskDelay(pdMS_TO_TICKS(us/1000));
  else {uint32_t t=micros();while(micros()-t<us) {}}
}
void taskIA(void*) {
  const uint32_t intervaloUs=(uint32_t)(EI_CLASSIFIER_INTERVAL_MS*1000.0f);
  for(;;) {
    for(size_t i=0;i<EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;i++) {
      features[i]=lerSensorEscalaIA();aguardarUs(intervaloUs);
    }
    signal_t signal;
    if(numpy::signal_from_buffer(features,EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,&signal)!=0) {
      iaReduzir=true;continue;
    }
    ei_impulse_result_t result={};
    if(run_classifier(&signal,&result,false)!=EI_IMPULSE_OK) {
      iaReduzir=true;Serial.println("Falha na inferencia");continue;
    }
    size_t melhor=0;
    for(size_t i=1;i<EI_CLASSIFIER_LABEL_COUNT;i++)
      if(result.classification[i].value>result.classification[melhor].value) melhor=i;
    const char* label=result.classification[melhor].label;
    float conf=result.classification[melhor].value;
    if(conf>=CONFIANCA_MINIMA) {
      iaReduzir=(strcasecmp(label,"reduzir")==0);
    } else iaReduzir=true; // baixa confiança: reduzir por segurança
    Serial.printf("IA=%s confianca=%.3f sensor=%.0f acao=%s\n",label,conf,
      features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE-1],iaReduzir?"REDUZIR":"NORMAL");
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
void setup() {
  Serial.begin(115200);analogReadResolution(12);pinMode(pinoSensor,INPUT);
  for(int p:{pinoIN_E1,pinoIN_E2,pinoIN_D1,pinoIN_D2})pinMode(p,OUTPUT);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(PinoPWM_Esquerdo,freq,resolucao);
  ledcAttach(PinoPWM_Direito,freq,resolucao);
#else
  ledcSetup(canalPWM_E,freq,resolucao);ledcSetup(canalPWM_D,freq,resolucao);
  ledcAttachPin(PinoPWM_Esquerdo,canalPWM_E);ledcAttachPin(PinoPWM_Direito,canalPWM_D);
#endif
  moverEsquerdo(0);moverDireito(0);
  BP32.setup(&onConnectedController,&onDisconnectedController);
  // Evitar BP32.forgetBluetoothKeys() a cada boot: apagaria pareamentos.
  Serial.println("CarrinhoLuz pronto: controle Bluetooth + TinyML");
  xTaskCreatePinnedToCore(taskIA,"taskIA",16384,nullptr,1,nullptr,0);
}
void loop() {
  if(BP32.update())processControllers();
  aplicarMotores();delay(20);
}
