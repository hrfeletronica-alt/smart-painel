# 🚁 Manual do Usuário • Smart Helipontos
### Sistema Inteligente de Controle de Balizamento Noturno e Iluminação de Solo (Foot Light)

---

## 1. Visão Geral do Sistema

O **Smart Helipontos** é uma solução desenvolvida para controle de iluminação aeronáutica em helipontos e heliportos. O sistema oferece acionamento híbrido:

1. **Local (Físico):** Através de botoeiras push-button instaladas diretamente na porta do painel elétrico.
2. **Remoto (Digital):** Através de aplicativo Web acessível por celular (4G/5G/Wi-Fi) ou computador, em qualquer lugar do mundo via Nuvem MQTT em tempo real.

> [!NOTE]
> **Operação Autônoma Garantida:** Os botões físicos do painel operam de forma 100% independente da internet. Caso a conexão Wi-Fi caia, o heliponto continuará funcionando normalmente através das botoeiras locais.

---

## 2. Controles do Balizamento e Iluminação

O sistema gerencia dois circuitos elétricos distintos: o **Balizamento Noturno de Pista** e o **Foot Light (Iluminação de Solo)**.

### 2.1. Balizamento de Pista (3 Estágios de Brilho)
O circuito de balizamento de perímetro conta com **intertravamento automático** (apenas um estágio pode estar ligado por vez, evitando sobrecarga ou conflito de fases):

| Estágio | Indicação Recomendada | Comportamento no Painel / App |
|:---:|:---|:---|
| **Brilho 1 (30%)** | Céu limpo, crepúsculo ou voos visuais noturnos padrão | Balizamento ativo em 30% de luminosidade |
| **Brilho 2 (70%)** | Neblina moderada, chuva leve ou maior contraste urbano | Balizamento ativo em 70% de luminosidade |
| **Brilho 3 (100%)** | Chuva forte, nevoeiro denso ou aproximações críticas | Balizamento ativo em 100% de intensidade |
| **Desligado (0%)** | Pista fora de operação | Balizamento totalmente desligado |

* **Lógica Liga/Desliga (Toggle):** Se o Estágio 2 já estiver aceso e você pressionar novamente o botão do Estágio 2, o balizamento será **desligado**. Para trocar de intensidade (ex: de 1 para 3), basta clicar direto no nível desejado.

### 2.2. Foot Light (Luz de Solo / Toque)
- **Operação 100% Independente:** A iluminação de solo para embarque/desembarque de passageiros e tripulação opera em circuito separado.
- Pode ser ligada ou desligada com o balizamento aceso ou com o balizamento totalmente desligado.

---

## 3. Operação pelos Botões Físicos do Painel

Na porta do painel elétrico estão localizados os 5 botões de comando:

```
 [ BOTÃO 1 ]   -> Liga / Alterna Balizamento Estágio 1 (30%)
 [ BOTÃO 2 ]   -> Liga / Alterna Balizamento Estágio 2 (70%)
 [ BOTÃO 3 ]   -> Liga / Alterna Balizamento Estágio 3 (100%)
 [ BOTÃO 4 ]   -> Liga / Desliga FOOT LIGHT (Solo)
 [ BOTÃO 5 ]   -> Reset de Wi-Fi (Segure 3 segundos para reconfigurar)
```

- Cada clique em um botão físico reflete **instantaneamente** na tela do celular de quem estiver com o aplicativo aberto.

---

## 4. Operação pelo Aplicativo Web / Celular

Você pode comandar o heliponto remotamente pelo celular ou tablet:

### 4.1. Como Acessar
1. Aponte a câmera do celular para o **QR Code** impresso no painel físico; ou
2. Acesse no navegador o endereço oficial:
   👉 **[https://hrfeletronica-alt.github.io/smart-painel/](https://hrfeletronica-alt.github.io/smart-painel/)**
3. Para helipontos específicos com ID cadastrado, adicione `?id=SEU-ID` no final do link (ex: `https://hrfeletronica-alt.github.io/smart-painel/?id=torre-sul`).

### 4.2. Recursos da Interface
- **Status em Tempo Real:** Mostra se o heliponto está Online ou Desconectado.
- **Botões Grandes de Toque:** Níveis 30%, 70%, 100% e Botão Desligar Balizamento.
- **Card Exclusivo Foot Light:** Botão liga/desliga com indicador visual luminoso.
- **Gerador de QR Code:** Permite compartilhar o acesso instantaneamente com o piloto, operador de rádio ou equipe de solo.

---

## 5. Como Trocar ou Configurar a Rede Wi-Fi

Caso o painel seja levado para outro heliponto ou a senha do roteador local tenha sido alterada:

1. **Abra o Modo de Configuração:**
   - No painel físico, **mantenha pressionado o Botão 5 (Reset) por 3 segundos**.
   - O Módulo criará uma rede Wi-Fi própria chamada:
     📶 **`SmartHeliponto-Config`**

2. **Conecte com o seu Celular:**
   - No celular, acerte as configurações de Wi-Fi e conecte-se na rede **`SmartHeliponto-Config`** (sem senha).
   - Uma tela de login abrirá automaticamente. Caso não abra, abra o navegador e digite:
     🌐 **`http://192.168.4.1`**

3. **Selecione a Rede e Salve:**
   - Toque em **Configure WiFi**.
   - Escolha o nome da rede Wi-Fi do heliponto e digite a senha.
   - *(Opcional)* Preencha o campo **ID do Heliponto** caso queira personalizar (ex: `heliponto-alphaville`).
   - Toque em **Save**.

4. **Pronto!**
   - O painel gravará a nova rede na memória permanente, desligará o modo de configuração e passará a operar normalmente conectado à nuvem.

---

## 6. Perguntas Frequentes (FAQ) & Solução de Problemas

#### A internet do prédio/condomínio caiu. O heliponto fica sem luz?
**Não.** Os botões físicos na porta do painel continuam funcionando normalmente e instantaneamente. Apenas o controle remoto pelo celular ficará pausado até que o roteador restabeleça o sinal.

#### Preciso estar no mesmo Wi-Fi para acionar pelo celular?
**Não.** A comunicação utiliza broker MQTT em nuvem global. Você pode acionar o balizamento estando no 4G/5G, no escritório ou em qualquer lugar do mundo.

#### O que significa "Timeout de 3 minutos no Portal de Configuração"?
Se alguém acionar o modo de configuração Wi-Fi por engano e ninguém conectar em até 3 minutos, o painel sai do modo de configuração e retorna para a operação normal das botoeiras.

#### O sistema pode pulsar ou dar falso disparo ao ligar a energia?
**Não.** O sistema conta com inicialização silenciosa ativa (*Hardware Safe Boot*), garantindo que todos os circuitos permaneçam 100% desligados até que haja uma ordem intencional do usuário.

---

## 7. Suporte Técnico e Garantia

Em caso de dúvidas técnicas, suporte à instalação ou manutenção:
- **Responsável Técnico:** HRF Eletrônica & Automação
- **Telefone / WhatsApp:** (11) 98960-9190
- **Repositório do Projeto:** [github.com/hrfeletronica-alt/smart-painel](https://github.com/hrfeletronica-alt/smart-painel)
