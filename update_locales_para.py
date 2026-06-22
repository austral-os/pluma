import json

def update_locale(path, strings):
    with open(path, 'r') as f:
        data = json.load(f)
    if "pluma-writer" not in data:
        data["pluma-writer"] = {}
    data["pluma-writer"]["paragraph_dialog"] = strings
    with open(path, 'w') as f:
        json.dump(data, f, indent=4, ensure_ascii=False)

en_strings = {
    "title": "Paragraph",
    "indents_spacing": "Indents & Spacing",
    "alignment": "Alignment",
    "indent": "Indent",
    "before_text": "Before text:",
    "after_text": "After text:",
    "first_line": "First line:",
    "spacing": "Spacing",
    "above_paragraph": "Above paragraph:",
    "below_paragraph": "Below paragraph:",
    "line_spacing": "Line Spacing",
    "ls_single": "Single",
    "ls_15": "1.5 lines",
    "ls_double": "Double",
    "ls_proportional": "Proportional"
}

es_strings = {
    "title": "Párrafo",
    "indents_spacing": "Sangrías y Espaciado",
    "alignment": "Alineación",
    "indent": "Sangría",
    "before_text": "Antes del texto:",
    "after_text": "Después del texto:",
    "first_line": "Primera línea:",
    "spacing": "Espaciado",
    "above_paragraph": "Sobre el párrafo:",
    "below_paragraph": "Debajo del párrafo:",
    "line_spacing": "Interlineado",
    "ls_single": "Sencillo",
    "ls_15": "1,5 líneas",
    "ls_double": "Doble",
    "ls_proportional": "Proporcional"
}

update_locale('/home/horacio/Desarrollo/pluma/locales/en.json', en_strings)
update_locale('/home/horacio/Desarrollo/pluma/locales/es.json', es_strings)
