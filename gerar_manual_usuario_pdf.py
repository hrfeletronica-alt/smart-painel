import os
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, HRFlowable, KeepTogether
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle

pdf_path = r"f:\Smart_painel\Manual_do_Usuario_Smart_Helipontos.pdf"
doc = SimpleDocTemplate(
    pdf_path,
    pagesize=A4,
    leftMargin=32,
    rightMargin=32,
    topMargin=28,
    bottomMargin=28
)

styles = getSampleStyleSheet()

# Paleta Visual Smart Helipontos
COR_PRIMARIA   = colors.HexColor('#17457c') # Azul Corporativo
COR_SECUNDARIA = colors.HexColor('#0284c7') # Azul Céu
COR_DESTAQUE   = colors.HexColor('#0f172a') # Grafite Escuro
COR_FUNDO_BOX  = colors.HexColor('#f8fafc') # Cinza Claro
COR_BORDA      = colors.HexColor('#cbd5e1') # Cinza Borda
COR_TEXTO      = colors.HexColor('#334155')

title_style = ParagraphStyle(
    'DocTitle',
    fontName='Helvetica-Bold',
    fontSize=15,
    leading=18,
    textColor=COR_PRIMARIA,
    alignment=0
)

subtitle_style = ParagraphStyle(
    'DocSubtitle',
    fontName='Helvetica-Bold',
    fontSize=8.5,
    leading=11,
    textColor=COR_SECUNDARIA,
    alignment=0
)

h2_style = ParagraphStyle(
    'H2',
    fontName='Helvetica-Bold',
    fontSize=10,
    leading=13,
    textColor=COR_DESTAQUE,
    spaceBefore=7,
    spaceAfter=3
)

body_style = ParagraphStyle(
    'Body',
    fontName='Helvetica',
    fontSize=8,
    leading=11.5,
    textColor=COR_TEXTO
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
    textColor=COR_DESTAQUE,
    alignment=1
)

table_cell_bold = ParagraphStyle(
    'CellBold',
    fontName='Helvetica-Bold',
    fontSize=7.5,
    leading=9.5,
    textColor=COR_DESTAQUE,
    alignment=1
)

table_cell_left = ParagraphStyle(
    'CellLeft',
    fontName='Helvetica',
    fontSize=7.5,
    leading=9.5,
    textColor=COR_TEXTO
)

elements = []

# Cabeçalho Principal
elements.append(Paragraph("SMART HELIPONTOS • MANUAL DO USUÁRIO & GUIA OPERACIONAL", title_style))
elements.append(Paragraph("SISTEMA DE CONTROLE DE BALIZAMENTO NOTURNO E FOOT LIGHT (ILUMINAÇÃO DE SOLO)", subtitle_style))
elements.append(Spacer(1, 4))
elements.append(HRFlowable(width="100%", thickness=2, color=COR_PRIMARIA, spaceAfter=6))

# 1. Apresentação Geral
elements.append(Paragraph("1. APRESENTAÇÃO DO SISTEMA", h2_style))
texto_apresentacao = """
O <b>Smart Helipontos</b> permite acionar o balizamento de pista e iluminação de toque tanto <b>manualmente pelo painel físico</b> quanto <b>remotamente pelo celular (4G/5G/Wi-Fi)</b>.<br/>
<b>• Autonomia Total:</b> Se a conexão com a internet cair, todas as botoeiras físicas continuam funcionando localmente em tempo real.<br/>
<b>• Sincronismo em Nuvem:</b> Qualquer acionamento feito nas botoeiras físicas é refletido instantaneamente na tela do aplicativo e vice-versa.
"""
elements.append(Paragraph(texto_apresentacao, body_style))
elements.append(Spacer(1, 4))

# 2. Tabela de Operação das Botoeiras Físicas
elements.append(Paragraph("2. OPERAÇÃO PELAS BOTOEIRAS FÍSICAS DO PAINEL", h2_style))

data_botoes = [
    [
        Paragraph("COMANDO FÍSICO", table_header_style),
        Paragraph("CIRCUITO / AÇÃO", table_header_style),
        Paragraph("INDICAÇÃO OPERACIONAL RECOMENDADA", table_header_style),
    ],
    [
        Paragraph("<b>BOTÃO 1</b>", table_cell_bold),
        Paragraph("<b>Balizamento 30%</b><br/>(Estágio 1)", table_cell_center),
        Paragraph("Céu limpo, crepúsculo ou voos visuais noturnos normais. Aperte novamente para desligar.", table_cell_left),
    ],
    [
        Paragraph("<b>BOTÃO 2</b>", table_cell_bold),
        Paragraph("<b>Balizamento 70%</b><br/>(Estágio 2)", table_cell_center),
        Paragraph("Neblina moderada, chuva leve ou maior contraste com iluminação urbana.", table_cell_left),
    ],
    [
        Paragraph("<b>BOTÃO 3</b>", table_cell_bold),
        Paragraph("<b>Balizamento 100%</b><br/>(Estágio 3)", table_cell_center),
        Paragraph("Nevoeiro denso, chuva intensa ou aproximações críticas de baixa visibilidade.", table_cell_left),
    ],
    [
        Paragraph("<b>BOTÃO 4</b>", table_cell_bold),
        Paragraph("<b>FOOT LIGHT</b><br/>(Iluminação de Solo)", table_cell_center),
        Paragraph("<b>100% Independente:</b> Iluminação de solo para embarque/desembarque. Liga/desliga livremente.", table_cell_left),
    ],
    [
        Paragraph("<b>RESET NO MÓDULO</b>", table_cell_bold),
        Paragraph("<b>Configuração Wi-Fi</b><br/>(Portal Cativo)", table_cell_center),
        Paragraph("<b>Segure por 3 segundos:</b> Abre a rede <i>SmartHeliponto-Config</i> para trocar o Wi-Fi pelo celular.", table_cell_left),
    ],
]

t_btn = Table(data_botoes, colWidths=[100, 130, 305])
t_btn.setStyle(TableStyle([
    ('BACKGROUND', (0, 0), (-1, 0), COR_PRIMARIA),
    ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
    ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
    ('GRID', (0, 0), (-1, -1), 0.5, COR_BORDA),
    ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.HexColor('#ffffff'), COR_FUNDO_BOX]),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 2.5),
    ('TOPPADDING', (0, 0), (-1, -1), 2.5),
]))
elements.append(t_btn)
elements.append(Spacer(1, 4))

# 3. Operação Remota via Celular / Web App
elements.append(Paragraph("3. OPERAÇÃO REMOTA PELO CELULAR OU COMPUTADOR (WEB APP)", h2_style))
texto_app = """
<b>• Acesso Instantâneo:</b> Aponte a câmera do celular para o <b>QR Code</b> impresso na tampa do painel, ou acesse no navegador: <b>https://hrfeletronica-alt.github.io/smart-painel/</b>.<br/>
<b>• Controle Global:</b> Funciona em qualquer rede (Wi-Fi, 4G, 5G) sem necessidade de estar no local.<br/>
<b>• Compartilhamento com Piloto/Operador:</b> O aplicativo permite gerar QR Codes rápidos para autorizar acesso temporário à tripulação.
"""
elements.append(Paragraph(texto_app, body_style))
elements.append(Spacer(1, 4))

# 4. Troca e Configuração de Nova Rede Wi-Fi em Campo
elements.append(Paragraph("4. COMO CONFIGURAR EM OUTRA REDE WI-FI (PASSO A PASSO)", h2_style))

data_wifi = [
    [
        Paragraph("PASSO", table_header_style),
        Paragraph("AÇÃO NO PAINEL / CELULAR", table_header_style),
        Paragraph("DETALHES / O QUE ESPERAR", table_header_style),
    ],
    [
        Paragraph("<b>Passo 1</b>", table_cell_bold),
        Paragraph("Segure o <b>Reset no Módulo por 3s</b>", table_cell_center),
        Paragraph("O Módulo cria a rede Wi-Fi própria: <b>SmartHeliponto-Config</b>.", table_cell_left),
    ],
    [
        Paragraph("<b>Passo 2</b>", table_cell_bold),
        Paragraph("Conecte seu celular na rede", table_cell_center),
        Paragraph("Conecte no Wi-Fi <b>SmartHeliponto-Config</b> (sem senha). A tela abrirá automaticamente.", table_cell_left),
    ],
    [
        Paragraph("<b>Passo 3</b>", table_cell_bold),
        Paragraph("Abra <b>http://192.168.4.1</b>", table_cell_center),
        Paragraph("Caso o navegador não abra sozinho, digite <b>192.168.4.1</b> no Chrome ou Safari.", table_cell_left),
    ],
    [
        Paragraph("<b>Passo 4</b>", table_cell_bold),
        Paragraph("Configure e Salve", table_cell_center),
        Paragraph("Toque em <i>Configure WiFi</i>, selecione a rede local, digite a senha e clique em <b>Save</b>.", table_cell_left),
    ],
]

t_wifi = Table(data_wifi, colWidths=[65, 165, 305])
t_wifi.setStyle(TableStyle([
    ('BACKGROUND', (0, 0), (-1, 0), COR_SECUNDARIA),
    ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
    ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
    ('GRID', (0, 0), (-1, -1), 0.5, COR_BORDA),
    ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.HexColor('#ffffff'), COR_FUNDO_BOX]),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 2.5),
    ('TOPPADDING', (0, 0), (-1, -1), 2.5),
]))
elements.append(t_wifi)
elements.append(Spacer(1, 4))

# 5. Dúvidas Frequentes (FAQ)
elements.append(Paragraph("5. DÚVIDAS FREQUENTES (FAQ) & SEGURANÇA OPERACIONAL", h2_style))
texto_faq = """
<b>• A internet caiu, o balizamento funciona?</b> Sim, 100% normal através dos botões físicos na porta do painel.<br/>
<b>• Posso acender o Foot Light com o balizamento ligado?</b> Sim, o Foot Light opera em circuito totalmente isolado e independente.<br/>
<b>• Intertravamento de Segurança:</b> O sistema nunca liga mais de um estágio de balizamento simultaneamente, protegendo lâmpadas e circuitos.<br/>
<b>• Partida Silenciosa:</b> Ao ligar o disjuntor geral, todos os circuitos iniciam obrigatoriamente desligados, prevenindo disparos falsos.
"""
elements.append(Paragraph(texto_faq, body_style))
elements.append(Spacer(1, 5))

# Rodapé Técnico
elements.append(HRFlowable(width="100%", thickness=1, color=COR_BORDA, spaceAfter=4))
rodape = Paragraph(
    "<font color='#64748b'>Smart Helipontos • Fabricação & Automação: HRF Eletrônica • Suporte Técnico: (11) 98960-9190 • Repositório: github.com/hrfeletronica-alt/smart-painel</font>",
    ParagraphStyle('Rodape', fontName='Helvetica', fontSize=7, alignment=1)
)
elements.append(rodape)

doc.build(elements)
print("Manual do Usuário em PDF gerado com sucesso em:", pdf_path)
