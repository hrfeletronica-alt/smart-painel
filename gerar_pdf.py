import os
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, HRFlowable
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle

pdf_path = r"f:\Smart_painel\Esquema_Ligacao_ESP32_DevKitV1_Reles.pdf"
doc = SimpleDocTemplate(
    pdf_path,
    pagesize=A4,
    leftMargin=30,
    rightMargin=30,
    topMargin=25,
    bottomMargin=25
)

styles = getSampleStyleSheet()

# Estilos Customizados
title_style = ParagraphStyle(
    'DocTitle',
    fontName='Helvetica-Bold',
    fontSize=16,
    leading=20,
    textColor=colors.HexColor('#17457c'),
    alignment=0
)

subtitle_style = ParagraphStyle(
    'DocSubtitle',
    fontName='Helvetica',
    fontSize=9,
    leading=12,
    textColor=colors.HexColor('#0284c7'),
    alignment=0
)

h2_style = ParagraphStyle(
    'H2',
    fontName='Helvetica-Bold',
    fontSize=10.5,
    leading=14,
    textColor=colors.HexColor('#0f172a'),
    spaceBefore=8,
    spaceAfter=4
)

body_style = ParagraphStyle(
    'Body',
    fontName='Helvetica',
    fontSize=8,
    leading=11.5,
    textColor=colors.HexColor('#334155')
)

table_header_style = ParagraphStyle(
    'TableHeader',
    fontName='Helvetica-Bold',
    fontSize=8,
    leading=10,
    textColor=colors.white,
    alignment=1
)

table_cell_center = ParagraphStyle(
    'CellCenter',
    fontName='Helvetica',
    fontSize=7.5,
    leading=9.5,
    textColor=colors.HexColor('#0f172a'),
    alignment=1
)

table_cell_bold = ParagraphStyle(
    'CellBold',
    fontName='Helvetica-Bold',
    fontSize=7.5,
    leading=9.5,
    textColor=colors.HexColor('#0f172a'),
    alignment=1
)

table_cell_left = ParagraphStyle(
    'CellLeft',
    fontName='Helvetica',
    fontSize=7.5,
    leading=9.5,
    textColor=colors.HexColor('#334155')
)

elements = []

# Cabeçalho Principal
elements.append(Paragraph("SMART HELIPONTOS • MANUAL TÉCNICO DE LIGAÇÃO", title_style))
elements.append(Paragraph("CONTROLADOR ESP32 DEVKIT V1 (ESP-WROOM-32) • BALIZAMENTO AERONÁUTICO + FOOT LIGHT AUXILIAR", subtitle_style))
elements.append(Spacer(1, 4))
elements.append(HRFlowable(width="100%", thickness=2, color=colors.HexColor('#17457c'), spaceAfter=8))

# 1. Tabela Principal de Pinagem dos Relés
elements.append(Paragraph("1. TABELA DE PINAGEM: ESP32 DEVKIT V1 ➔ MÓDULO DE RELÉS", h2_style))

data_pinos = [
    [
        Paragraph("CIRCUITO / ESTÁGIO", table_header_style),
        Paragraph("PINO ESP32", table_header_style),
        Paragraph("ENTRADA RELÉ", table_header_style),
        Paragraph("SAÍDA FÍSICA", table_header_style),
        Paragraph("FUNÇÃO OPERACIONAL", table_header_style),
    ],
    [
        Paragraph("<b>BRILHO 1</b><br/>(30% Intensidade)", table_cell_bold),
        Paragraph("<b>GPIO 18</b> (D18)", table_cell_bold),
        Paragraph("IN 1", table_cell_center),
        Paragraph("Relé 1 (NO1 / COM1)", table_cell_center),
        Paragraph("Balizamento Pista - Nível Baixo (30%)", table_cell_left),
    ],
    [
        Paragraph("<b>BRILHO 2</b><br/>(70% Intensidade)", table_cell_bold),
        Paragraph("<b>GPIO 19</b> (D19)", table_cell_bold),
        Paragraph("IN 2", table_cell_center),
        Paragraph("Relé 2 (NO2 / COM2)", table_cell_center),
        Paragraph("Balizamento Pista - Nível Médio (70%)", table_cell_left),
    ],
    [
        Paragraph("<b>BRILHO 3</b><br/>(100% Intensidade)", table_cell_bold),
        Paragraph("<b>GPIO 21</b> (D21)", table_cell_bold),
        Paragraph("IN 3", table_cell_center),
        Paragraph("Relé 3 (NO3 / COM3)", table_cell_center),
        Paragraph("Balizamento Pista - Nível Máximo (100%)", table_cell_left),
    ],
    [
        Paragraph("<b>FOOT LIGHT</b><br/>(Iluminação Solo)", table_cell_bold),
        Paragraph("<b>GPIO 22</b> (D22)", table_cell_bold),
        Paragraph("IN 4", table_cell_center),
        Paragraph("Relé 4 (NO4 / COM4)", table_cell_center),
        Paragraph("Iluminação Auxiliar de Solo (Circuito Independente)", table_cell_left),
    ],
    [
        Paragraph("<b>DESABILITADO</b>", table_cell_center),
        Paragraph("GPIO 25 (D25)", table_cell_center),
        Paragraph("IN 5", table_cell_center),
        Paragraph("Relé 5", table_cell_center),
        Paragraph("Desabilitado no firmware (reserva técnica)", table_cell_left),
    ],
    [
        Paragraph("<b>DESABILITADO</b>", table_cell_center),
        Paragraph("GPIO 26 (D26)", table_cell_center),
        Paragraph("IN 6", table_cell_center),
        Paragraph("Relé 6", table_cell_center),
        Paragraph("Desabilitado no firmware (reserva técnica)", table_cell_left),
    ],
]

t1 = Table(data_pinos, colWidths=[95, 80, 65, 100, 195])
t1.setStyle(TableStyle([
    ('BACKGROUND', (0, 0), (-1, 0), colors.HexColor('#17457c')),
    ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
    ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
    ('GRID', (0, 0), (-1, -1), 0.5, colors.HexColor('#cbd5e1')),
    ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.HexColor('#ffffff'), colors.HexColor('#f8fafc')]),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 3),
    ('TOPPADDING', (0, 0), (-1, -1), 3),
]))
elements.append(t1)
elements.append(Spacer(1, 6))

# 2. Tabela dos 4 Botões Físicos (Push Buttons / Botoeiras)
elements.append(Paragraph("2. TABELA DAS 4 BOTOEIRAS FÍSICAS DO PAINEL (PUSH BUTTONS)", h2_style))

data_botoes = [
    [
        Paragraph("BOTOEIRA FÍSICA", table_header_style),
        Paragraph("PINO ESP32", table_header_style),
        Paragraph("SEGUNDO TERMINAL", table_header_style),
        Paragraph("COMPORTAMENTO OPERACIONAL", table_header_style),
    ],
    [
        Paragraph("<b>BOTÃO 1</b> (Brilho 1)", table_cell_bold),
        Paragraph("<b>GPIO 32</b> (D32)", table_cell_bold),
        Paragraph("GND (Terra Comum)", table_cell_center),
        Paragraph("Liga / Desliga <b>Brilho 1 (30%)</b> • Aciona Relé 1", table_cell_left),
    ],
    [
        Paragraph("<b>BOTÃO 2</b> (Brilho 2)", table_cell_bold),
        Paragraph("<b>GPIO 33</b> (D33)", table_cell_bold),
        Paragraph("GND (Terra Comum)", table_cell_center),
        Paragraph("Liga / Desliga <b>Brilho 2 (70%)</b> • Aciona Relé 2", table_cell_left),
    ],
    [
        Paragraph("<b>BOTÃO 3</b> (Brilho 3)", table_cell_bold),
        Paragraph("<b>GPIO 27</b> (D27)", table_cell_bold),
        Paragraph("GND (Terra Comum)", table_cell_center),
        Paragraph("Liga / Desliga <b>Brilho 3 (100%)</b> • Aciona Relé 3", table_cell_left),
    ],
    [
        Paragraph("<b>BOTÃO 4</b> (Foot Light)", table_cell_bold),
        Paragraph("<b>GPIO 14</b> (D14)", table_cell_bold),
        Paragraph("GND (Terra Comum)", table_cell_center),
        Paragraph("Liga / Desliga <b>FOOT LIGHT</b> • Aciona Relé 4 (Independente)", table_cell_left),
    ],
]

t_btn = Table(data_botoes, colWidths=[105, 85, 110, 235])
t_btn.setStyle(TableStyle([
    ('BACKGROUND', (0, 0), (-1, 0), colors.HexColor('#0284c7')),
    ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
    ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
    ('GRID', (0, 0), (-1, -1), 0.5, colors.HexColor('#cbd5e1')),
    ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.HexColor('#ffffff'), colors.HexColor('#f8fafc')]),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 3),
    ('TOPPADDING', (0, 0), (-1, -1), 3),
]))
elements.append(t_btn)
elements.append(Spacer(1, 6))

# 3. Alimentação e Isolamento
elements.append(Paragraph("3. ESQUEMA DE ALIMENTAÇÃO, ISOLAMENTO E ATERRAMENTO", h2_style))

data_alimentacao = [
    [
        Paragraph("CONEXÃO", table_header_style),
        Paragraph("ORIGEM", table_header_style),
        Paragraph("DESTINO", table_header_style),
        Paragraph("OBSERVAÇÕES TÉCNICAS E BOAS PRÁTICAS", table_header_style),
    ],
    [
        Paragraph("<b>GND COMUM</b>", table_cell_bold),
        Paragraph("GND do ESP32", table_cell_center),
        Paragraph("GND da Placa de Relés e Fonte", table_cell_center),
        Paragraph("Interligar todos os terras para referência lógica comum estável.", table_cell_left),
    ],
    [
        Paragraph("<b>ALIMENTAÇÃO ESP32</b>", table_cell_bold),
        Paragraph("Fonte 5V DC (2A) ou USB", table_cell_center),
        Paragraph("Pino VIN do ESP32", table_cell_center),
        Paragraph("Regulador interno do ESP32 gera os 3.3V com filtragem de ruído.", table_cell_left),
    ],
    [
        Paragraph("<b>VCC / JD-VCC</b>", table_cell_bold),
        Paragraph("Fonte Externa 5V DC", table_cell_center),
        Paragraph("Pino VCC / JD-VCC do Relé", table_cell_center),
        Paragraph("Garante corrente suficiente para acionamento simultâneo das bobinas sem queda no microcontrolador.", table_cell_left),
    ],
    [
        Paragraph("<b>CHAVEAMENTO SELETOR H/L</b>", table_cell_bold),
        Paragraph("Jumper da Placa Relé", table_cell_center),
        Paragraph("Posição L (Low) ou H (High)", table_cell_center),
        Paragraph("Firmware configurado em Active LOW (LOW liga, HIGH desliga) com boot silencioso.", table_cell_left),
    ],
]

t2 = Table(data_alimentacao, colWidths=[105, 95, 115, 220])
t2.setStyle(TableStyle([
    ('BACKGROUND', (0, 0), (-1, 0), colors.HexColor('#0f172a')),
    ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
    ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
    ('GRID', (0, 0), (-1, -1), 0.5, colors.HexColor('#cbd5e1')),
    ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.HexColor('#ffffff'), colors.HexColor('#f8fafc')]),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 3),
    ('TOPPADDING', (0, 0), (-1, -1), 3),
]))
elements.append(t2)
elements.append(Spacer(1, 6))

# 4. Regras de Funcionamento do Firmware
elements.append(Paragraph("4. LÓGICA DE FUNCIONAMENTO (FIRMWARE ESP32)", h2_style))
regras_texto = """
<b>• 1 Relé por Estágio de Balizamento:</b> Relé 1 (30%), Relé 2 (70%) e Relé 3 (100%) operam com intertravamento eletrônico automático (ao acionar um nível, o anterior é desligado instantaneamente).<br/>
<b>• Foot Light 100% Independente:</b> O Relé 4 (GPIO 22 / Botão GPIO 14) pode ser ligado ou desligado a qualquer momento, mantendo o balizamento de pista ativo ou desligado.<br/>
<b>• Boot Silencioso (Zero Disparos):</b> O microcontrolador inicializa os pinos de relé em nível DESLIGADO no primeiro ciclo de instrução do setup, prevenindo batimento ou pulso falso ao ligar/reiniciar.<br/>
<b>• Controle Simultâneo Físico e Nuvem:</b> Opera em tempo real tanto pelos 4 botões físicos com debounce de ruído, quanto pela Web/Celular via MQTT Nuvem (broker.emqx.io) de qualquer rede 4G/5G.
"""
elements.append(Paragraph(regras_texto, body_style))
elements.append(Spacer(1, 6))

# Rodapé Técnico
elements.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor('#cbd5e1'), spaceAfter=4))
rodape = Paragraph(
    "<font color='#64748b'>Smart Helipontos • Suporte Técnico: (11) 98960-9190 • Repositório: github.com/hrfeletronica-alt/smart-painel</font>",
    ParagraphStyle('Rodape', fontName='Helvetica', fontSize=7, alignment=1)
)
elements.append(rodape)

doc.build(elements)
print("PDF gerado com sucesso em:", pdf_path)
