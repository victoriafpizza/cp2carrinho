# CP2 — Carrinho com TinyML | CarrinhoLuz

Victoria Franceschini Pizza 
rm550609

## Projeto
Integração de controle por Bluetooth (Bluepad32), sensor analógico GPIO39, inferência Edge Impulse no ESP32 e redução de velocidade baseada na classificação **reduzir**. Classes do projeto: **avancar** e **reduzir**.

- **Edge Impulse público:** https://studio.edgeimpulse.com/public/1145521/live
- **Projeto/ID:** CarrinhoLuz / 1145521 (HilliKoala)
- **Modelo incluído?** NÃO. A biblioteca `CarrinhoLuz_inferencing` precisa ser exportada do projeto autorizado e instalada na IDE Arduino. O cabeçalho no sketch não substitui a biblioteca.


## Sensor, dados e coleta (10 pontos)
O firmware fornecido lê **um sensor analógico não identificado no enunciado** no GPIO39 (ADC1_CH3). Não é possível afirmar o modelo exato do sensor sem identificação física. Valores lidos em ADC 12 bits (0–4095) são escalados para 0–1023 antes de serem usados na IA.

A página pública do Edge Impulse informa **9 amostras / 47 segundos de dados**, eixo **Valor_sensor**, **100 Hz**, classes `avancar` e `reduzir`; distribuição **7 treinamento / 2 teste (79%/21%)**. Esses dados são metadados publicados, e **não são CSVs de coleta bruta**. Não foi possível obter o download dos arquivos de amostras neste ambiente. Os CSVs deverão ser exportados do Edge Impulse ou coletados pelo grupo para integrar esta pasta.

Para coletar: grave `firmware/coleta_analogica/coleta_analogica.ino`, abra o Serial a 115200 e use `scripts/coletar_serial_csv.py --porta COM3 --classe avancar --segundos 20 --saida dataset/avancar_01.csv` (troque COM3); repita com `reduzir` em condição apropriada. **Não apresente amostras sintéticas como dados medidos.**

## Edge Impulse (15 pontos)
1. Consulte o projeto público no link acima; confira os dados em Data acquisition e seu split.
2. Abra Impulse design e revise **tamanho da janela**, **frequência de 100 Hz**, blocos de processamento e classificação, registrando capturas das telas. As opções internas exatas não foram verificadas aqui.
3. Confira treinamento, matriz de confusão, model testing e Live classification; salve capturas autênticas em `evidencias/edge_impulse/`.
4. O painel público apresenta **100,0% validação** e **97,1% teste** para modelo **float32 não otimizado**; indica no alvo ESP-EYE com **quantizado int8** latência 1 ms, pico de RAM 1,4 KB, flash 14,4 KB. São números exibidos pelo Edge Impulse, **não resultados de teste do carrinho físico**. Acurácia num conjunto muito pequeno não garante generalização.
5. Exporte em **Deployment → Arduino library**, instale ZIP pelo Arduino IDE e confirme a presença de `CarrinhoLuz_inferencing.h`.

## Integração ESP32 (10 pontos)
Sketch de referência compatibilizado está em `firmware/carrinho_integrado/carrinho_integrado.ino`. Mapeamento: sensor GPIO39; esquerda PWM19 / IN18 / IN5; direita PWM15 / IN4 / IN2. A classificação `reduzir` com confiança >= 0,70 reduz velocidade para 60% do comando; `avancar` mantém velocidade. Segurança: motor só recebe comandos quando controle conectado e com dados recentes; classificação ausente/erro passa a reduzir preventivamente. Leia `docs/ALTERACOES_CODIGO.md`.

**Importante:** Bluepad32 requer instalação/placa compatível, e a biblioteca de inferência precisa ser gerada pelo projeto. Teste as versões ESP32 core e Bluepad32 com hardware real; estas alterações não foram compiladas ou executadas fisicamente aqui. Teste com as rodas suspensas e confirme conexões/fonte/polaridade antes de apoiar o carrinho no chão.

## Vídeo (60 pontos)
`video/demonstracao_recebida.mp4`: vídeo original enviado pelo grupo, incluído sem edição. **Não foi verificado que o vídeo demonstra a leitura, a inferência e a reação física exigidas.** Apresente leitura, rótulo/confiança, reação observável e consistência em repetições. Duração do arquivo: ~66,35 s.

## Organização
`docs/RUBRICA_E_CHECKLIST.md` separa evidência pronta do que falta validar. Em `docs/REGISTRO_TESTES.md` há modelo para anotar testes físicos (sem alegar resultados não obtidos). `docs/COMMITS_CP1_CP2.md` sugere evolução; o ZIP não cria commits nem publica no GitHub.

## Créditos
Integrantes: preencher nomes/RMs reais no `docs/INTEGRANTES.md`.

## Fonte
Página pública CarrinhoLuz acessada em 21/10/2026: https://studio.edgeimpulse.com/public/1145521/live
