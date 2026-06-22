import json

def update_locale(path, strings):
    with open(path, 'r') as f:
        data = json.load(f)
    if "pluma-writer" not in data:
        data["pluma-writer"] = {}
    data["pluma-writer"]["font_dialog"] = strings
    with open(path, 'w') as f:
        json.dump(data, f, indent=4, ensure_ascii=False)

en_strings = {
    "character": "Character",
    "family": "Family:",
    "style": "Style:",
    "size": "Size:",
    "language": "Language:",
    "font": "Font",
    "font_effects": "Font Effects",
    "position": "Position",
    "highlighting": "Highlighting",
    "borders": "Borders",
    "ok": "OK",
    "cancel": "Cancel",
    "regular": "Regular",
    "italic": "Italic",
    "bold": "Bold",
    "bold_italic": "Bold Italic"
}

es_strings = {
    "character": "Carácter",
    "family": "Familia:",
    "style": "Estilo:",
    "size": "Tamaño:",
    "language": "Idioma:",
    "font": "Fuente",
    "font_effects": "Efectos de Fuente",
    "position": "Posición",
    "highlighting": "Resaltado",
    "borders": "Bordes",
    "ok": "Aceptar",
    "cancel": "Cancelar",
    "regular": "Regular",
    "italic": "Cursiva",
    "bold": "Negrita",
    "bold_italic": "Negrita Cursiva"
}

update_locale('/home/horacio/Desarrollo/pluma/locales/en.json', en_strings)
update_locale('/home/horacio/Desarrollo/pluma/locales/es.json', es_strings)
