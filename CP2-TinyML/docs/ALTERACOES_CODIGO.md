# Revisão técnica do código enviado

O firmware integrado preserva o comportamento geral do código recebido, com correções destacadas:

1. **ESP32 Core 3.x:** `ledcWrite()` recebe o **pino PWM** (19/15) em vez de canal 0/1; no Core 2.x continua por canal. No código enviado a escrita usava o canal nas duas versões.
2. **Watchdog do controle:** 400 ms sem atualização dos comandos zera a velocidade para evitar movimento descontrolado na perda de comunicação.
3. **Falha ou baixa confiança:** fator reduzido por precaução, sem inferir ação `avancar` a partir de resultado incerto.
4. **Bluetooth:** não apaga pareamentos na inicialização, salvo necessidade explícita.
5. **Sem mudança no mapeamento de hardware recebido.**
6. **O código recebido estava sem dados suficientes para confirmar a identidade do sensor**; o projeto é `CarrinhoLuz` e usa sensor **analógico**, não assumir HC-SR04.

**Atenção:** o firmware alterado precisa ser compilado/testado com a placa ESP32 e a biblioteca concreta exportada do Edge Impulse. Não há evidência de execução verificada neste pacote.
