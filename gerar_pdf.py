import os
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, HRFlowable
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle

pdf_path = r"f:\Smart_painel\Esquema_Ligacao_ESP32_DevKitV1_Reles.pdf"
doc = SimpleDocTemplate(
    pdf_path,
    pagesize=A4,
    leftMargin=35,
    rightMargin=35,
    topMargin=35,
    bottomMargin=35
)

styles = getSampleStyleSheet()

# Estilos Customizados
title_style = ParagraphStyle(
    'DocTitle',
    fontName='Helvetica-Bold',
    fontSize=18,
    leading=22,
    textColor=colors.HexColor('#17457c'),
    alignment=0
)

subtitle_style = ParagraphStyle(
    'DocSubtitle',
    fontName='Helvetica',
    fontSize=10,
    leading=14,
    textColor=colors.HexColor('#0284c7'),
    alignment=0
)

h2_style = ParagraphStyle(
    'H2',
    fontName='Helvetica-Bold',
    fontSize=12,
    leading=16,
    textColor=colors.HexColor('#0f172a'),
    spaceBefore=10,
    spaceAfter=6
)

body_style = ParagraphStyle(
    'Body',
    fontName='Helvetica',
    fontSize=9,
    leading=13,
    textColor=colors.HexColor('#334155')
)

body_bold = ParagraphStyle(
    'BodyBold',
    fontName='Helvetica-Bold',
    fontSize=9,
    leading=13,
    textColor=colors.HexColor('#0f172a')
)

table_header_style = ParagraphStyle(
    'TableHeader',
    fontName='Helvetica-Bold',
    fontSize=9,
    leading=12,
    textColor=colors.white,
    alignment=1
)

table_cell_center = ParagraphStyle(
    'CellCenter',
    fontName='Helvetica',
    fontSize=8.5,
    leading=11,
    textColor=colors.HexColor('#0f172a'),
    alignment=1
)

table_cell_bold = ParagraphStyle(
    'CellBold',
    fontName='Helvetica-Bold',
    fontSize=8.5,
    leading=11,
    textColor=colors.HexColor('#0f172a'),
    alignment=1
)

table_cell_left = ParagraphStyle(
    'CellLeft',
    fontName='Helvetica',
    fontSize=8.5,
    leading=11,
    textColor=colors.HexColor('#334155')
)

elements = []

# Cabeçalho Principal
elements.append(Paragraph("( PAINEL ) SMART HELIPONTOS", title_style))
elements.append(Paragraph("MANUAL TÉCNICO DE LIGAÇÃO • CONTROLADOR ESP32 DEVKIT V1 (ESP-WROOM-32) COM MÓDULO 6 RELÉS", subtitle_style))
elements.append(Spacer(1, 8))
elements.append(HRFlowable(width="100%", thickness=2, color=colors.HexColor('#17457c'), spaceAfter=12))

# 1. Tabela Principal de Pinagem
elements.append(Paragraph("1. TABELA DE PINAGEM: ESP32 DEVKIT V1 ➔ MÓDULO 6 RELÉS", h2_style))

data_pinos = [
    [
        Paragraph("CANAL / ESTÁGIO", table_header_style),
        Paragraph("PINO ESP32", table_header_style),
        Paragraph("ENTRADA RELÉ", table_header_style),
        Paragraph("SAÍDA FÍSICA", table_header_style),
        Paragraph("FUNÇÃO AERONÁUTICA", table_header_style),
    ],
    [
        Paragraph("<b>BRILHO 1</b><br/>(30% Intensidade)", table_cell_bold),
        Paragraph("<b>GPIO 18</b> (D18)", table_cell_bold),
        Paragraph("IN 1", table_cell_center),
        Paragraph("Relé 1 (NO1/COM1)", table_cell_center),
        Paragraph("Balizamento Pista - Potência Baixa (Circuito A)", table_cell_left),
    ],
    [
        Paragraph("<b>BRILHO 1</b><br/>(30% Intensidade)", table_cell_bold),
        Paragraph("<b>GPIO 19</b> (D19)", table_cell_bold),
        Paragraph("IN 2", table_cell_center),
        Paragraph("Relé 2 (NO2/COM2)", table_cell_center),
        Paragraph("Balizamento Pista - Potência Baixa (Circuito B)", table_cell_left),
    ],
    [
        Paragraph("<b>BRILHO 2</b><br/>(70% Intensidade)", table_cell_bold),
        Paragraph("<b>GPIO 21</b> (D21)", table_cell_bold),
        Paragraph("IN 3", table_cell_center),
        Paragraph("Relé 3 (NO3/COM3)", table_cell_center),
        Paragraph("Balizamento Pista - Potência Média (Circuito A)", table_cell_left),
    ],
    [
        Paragraph("<b>BRILHO 2</b><br/>(70% Intensidade)", table_cell_bold),
        Paragraph("<b>GPIO 22</b> (D22)", table_cell_bold),
        Paragraph("IN 4", table_cell_center),
        Paragraph("Relé 4 (NO4/COM4)", table_cell_center),
        Paragraph("Balizamento Pista - Potência Média (Circuito B)", table_cell_left),
    ],
    [
        Paragraph("<b>BRILHO 3</b><br/>(100% Intensidade)", table_cell_bold),
        Paragraph("<b>GPIO 25</b> (D25)", table_cell_bold),
        Paragraph("IN 5", table_cell_center),
        Paragraph("Relé 5 (NO5/COM5)", table_cell_center),
        Paragraph("Balizamento Pista - Potência Máxima (Circuito A)", table_cell_left),
    ],
    [
        Paragraph("<b>BRILHO 3</b><br/>(100% Intensidade)", table_cell_bold),
        Paragraph("<b>GPIO 26</b> (D26)", table_cell_bold),
        Paragraph("IN 6", table_cell_center),
        Paragraph("Relé 6 (NO6/COM6)", table_cell_center),
        Paragraph("Balizamento Pista - Potência Máxima (Circuito B)", table_cell_left),
    ],
]

t1 = Table(data_pinos, colWidths=[90, 85, 70, 95, 185])
t1.setStyle(TableStyle([
    ('BACKGROUND', (0, 0), (-1, 0), colors.HexColor('#17457c')),
    ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
    ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
    ('GRID', (0, 0), (-1, -1), 0.5, colors.HexColor('#cbd5e1')),
    ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.HexColor('#ffffff'), colors.HexColor('#f8fafc')]),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 5),
    ('TOPPADDING', (0, 0), (-1, -1), 5),
]))
elements.append(t1)
elements.append(Spacer(1, 12))

# 2. Tabela dos 3 Botões Físicos (Push Buttons / Botoeiras)
elements.append(Paragraph("2. TABELA DOS 3 BOTÕES FÍSICOS (BOTOEIRAS DO PAINEL)", h2_style))

data_botoes = [
    [
        Paragraph("BOTÃO FÍSICO", table_header_style),
        Paragraph("PINO ESP32", table_header_style),
        Paragraph("OUTRO TERMINAL DO BOTÃO", table_header_style),
        Paragraph("FUNÇÃO / AÇÃO", table_header_style),
    ],
    [
        Paragraph("<b>BOTAO 1</b><br/>(Push Button)", table_cell_bold),
        Paragraph("<b>GPIO 32</b> (D32)", table_cell_bold),
        Paragraph("GND (Terra)", table_cell_center),
        Paragraph("Aciona / Desliga <b>Brilho 1 (30%)</b> • Relés 1 e 2", table_cell_left),
    ],
    [
        Paragraph("<b>BOTAO 2</b><br/>(Push Button)", table_cell_bold),
        Paragraph("<b>GPIO 33</b> (D33)", table_cell_bold),
        Paragraph("GND (Terra)", table_cell_center),
        Paragraph("Aciona / Desliga <b>Brilho 2 (70%)</b> • Relés 3 e 4", table_cell_left),
    ],
    [
        Paragraph("<b>BOTAO 3</b><br/>(Push Button)", table_cell_bold),
        Paragraph("<b>GPIO 27</b> (D27)", table_cell_bold),
        Paragraph("GND (Terra)", table_cell_center),
        Paragraph("Aciona / Desliga <b>Brilho 3 (100%)</b> • Relés 5 e 6", table_cell_left),
    ],
]

t_btn = Table(data_botoes, colWidths=[100, 100, 130, 195])
t_btn.setStyle(TableStyle([
    ('BACKGROUND', (0, 0), (-1, 0), colors.HexColor('#0284c7')),
    ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
    ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
    ('GRID', (0, 0), (-1, -1), 0.5, colors.HexColor('#cbd5e1')),
    ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.HexColor('#ffffff'), colors.HexColor('#f8fafc')]),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 5),
    ('TOPPADDING', (0, 0), (-1, -1), 5),
]))
elements.append(t_btn)
elements.append(Spacer(1, 10))

# 3. Alimentação e Aterramento
elements.append(Paragraph("3. ESQUEMA DE ALIMENTAÇÃO E ISOLAMENTO (CRÍTICO)", h2_style))

data_alimentacao = [
    [
        Paragraph("CONEXÃO", table_header_style),
        Paragraph("ORIGEM", table_header_style),
        Paragraph("DESTINO", table_header_style),
        Paragraph("OBSERVAÇÕES TÉCNICAS", table_header_style),
    ],
    [
        Paragraph("<b>GND COMUM</b>", table_cell_bold),
        Paragraph("GND do ESP32", table_cell_center),
        Paragraph("GND do Módulo Relé e da Fonte", table_cell_center),
        Paragraph("<b>Obrigatório:</b> Interligar todos os GNDs para referência lógica estável.", table_cell_left),
    ],
    [
        Paragraph("<b>VCC LÓGICO</b>", table_cell_bold),
        Paragraph("Pino 3V3 do ESP32", table_cell_center),
        Paragraph("VCC do Módulo Relé", table_cell_center),
        Paragraph("Alimenta os optoacopladores do módulo (nível lógico 3.3V).", table_cell_left),
    ],
    [
        Paragraph("<b>JD-VCC (BOBINAS)</b>", table_cell_bold),
        Paragraph("Fonte Externa 5V (2A)", table_cell_center),
        Paragraph("Pino JD-VCC do Relé", table_cell_center),
        Paragraph("<b>Remover o jumper VCC-JDVCC</b> para isolamento galvânico total.", table_cell_left),
    ],
    [
        Paragraph("<b>ALIMENTAÇÃO ESP32</b>", table_cell_bold),
        Paragraph("Fonte 5V Externa ou USB", table_cell_center),
        Paragraph("Pino VIN do ESP32", table_cell_center),
        Paragraph("Permite ligar o ESP32 diretamente na mesma fonte 5V das bobinas.", table_cell_left),
    ],
]

t2 = Table(data_alimentacao, colWidths=[110, 115, 120, 180])
t2.setStyle(TableStyle([
    ('BACKGROUND', (0, 0), (-1, 0), colors.HexColor('#0284c7')),
    ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
    ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
    ('GRID', (0, 0), (-1, -1), 0.5, colors.HexColor('#cbd5e1')),
    ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.HexColor('#ffffff'), colors.HexColor('#f8fafc')]),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 5),
    ('TOPPADDING', (0, 0), (-1, -1), 5),
]))
elements.append(t2)
elements.append(Spacer(1, 12))

# 3. Regras de Funcionamento do Firmware
elements.append(Paragraph("3. LÓGICA DE FUNCIONAMENTO (FIRMWARE GRAVADO)", h2_style))
regras_texto = """
<b>• Intertravamento Rígido:</b> Nunca mais de 2 relés ativos simultaneamente. Ao acionar um estágio diferente pelo PC ou celular, os 2 relés anteriores são desligados na mesma fração de segundo antes de comutar para o novo nível.<br/>
<b>• Toggle Liga/Desliga:</b> Clicar no mesmo estágio ativo desliga o balizamento por completo.<br/>
<b>• Acionamento de Qualquer Lugar do Mundo:</b> Opera através do protocolo industrial MQTT via Nuvem (broker.emqx.io) em tempo real, sem necessidade de abertura de portas no roteador ou IP fixo.<br/>
<b>• Pinagem Anti-Boot Spike:</b> Os GPIOs 18, 19, 21, 22, 25 e 26 foram selecionados por serem imunes a transientes de boot do ESP32, garantindo que os relés não disparem acidentalmente em quedas de energia.
"""
elements.append(Paragraph(regras_texto, body_style))
elements.append(Spacer(1, 12))

# Rodapé Técnico
elements.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor('#cbd5e1'), spaceAfter=8))
rodape = Paragraph(
    "<font color='#64748b'>Smart Helipontos • Suporte Técnico: (11) 98960-9190 • Repositório: github.com/hrfeletronica-alt/smart-painel</font>",
    ParagraphStyle('Rodape', fontName='Helvetica', fontSize=7.5, alignment=1)
)
elements.append(rodape)

doc.build(elements)
print("PDF gerado com sucesso em:", pdf_path)
