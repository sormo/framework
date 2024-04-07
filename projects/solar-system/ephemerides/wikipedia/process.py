import requests, json, re
from bs4 import BeautifulSoup

jupiter_groups = {
    'Inner': ' ',
    'Galilean': '\u2660',
    'Themisto': '\u2020',
    'Himalaia': '\u2663',
    'Carpo': '\u00a7',
    'Valetudo': '\u00b1',
    'Ananke': '\u2666',
    'Pasiphae': '\u2021',
    'Carme': '\u2665'
}

saturn_groups = {
    'Major': '♠',
    'Inuit': '♦',
    'Gallic': '♣',
    'Norse': '‡',
    'Outlier': '§'
}

uranus_groups = {
    'Major': '♠',
    'Prograde': '†',
    'Retrograde': '‡',
    'Caliban': '♦'
}

neptune_groups = {
    'Triton': '♠',
    'Nereid': '†',
    'Halimede': '‡',
    'Sao': '♦',
    'Neso': '♥'
}

def clear_name(name):
    name = name.replace('\u2660', ' ')
    name = name.replace('\u2020', ' ')
    name = name.replace('\u2663', ' ')
    name = name.replace('\u00a7', ' ')
    name = name.replace('\u00b1', ' ')
    name = name.replace('\u2666', ' ')
    name = name.replace('\u2665', ' ')
    name = name.replace('\u2021', ' ')

    return name.strip()

def get_diameter_and_dimensions(diameter):
    diameter = diameter.replace('\u2009', '')
    diameter = diameter.replace('\u2248', '')
    
    p1 = diameter.find('\xa0')
    if p1 == -1:
        p1 = diameter.find('±')
    if p1 == -1:
        p1 = diameter.find('+')
    if p1 == -1:
        p1 = diameter.find('-')
    if p1 != -1:
        p2 = diameter.find('(')
        if p2 != -1:
            diameter = diameter[0:p1] + diameter[p2:]
        else:
            diameter = diameter[0:p1]

    o = re.match(r'([0-9.]*)\((.*)\)', diameter)
    if o:
        diameter_value = float(o.group(1))
        dimensions = o.group(2)
        dimensions = dimensions.replace('\u2009', ' ')
        dimensions = dimensions.replace('\u00d7', ' ')
        dimensions_list = dimensions.split()
        if len(dimensions_list) == 3:
            dimensions_value = (float(dimensions_list[0]), float(dimensions_list[1]), float(dimensions_list[2]))
        else:
            dimensions_value = (float(dimensions_list[0]), float(dimensions_list[1]))
        return diameter_value, dimensions_value
    else:
        return float(diameter), ()

def get_mass_value(mass, exponent):
    mass = mass.replace('\u2248', ' ')
    mass = mass.replace('\u2009', ' ')
    mass.strip()

    if '±' in mass:
        mass = mass[:mass.find('±')]

    mass += exponent
    return float(mass)

def process_jupiter_moons():
    url = "https://en.wikipedia.org/wiki/Moons_of_Jupiter"

    response = requests.get(url)

    # Parse the HTML content
    soup = BeautifulSoup(response.text, 'html.parser')

    # Find the table you want to parse
    tables = soup.find_all('table', {'class': 'wikitable'})

    result = []
    table = tables[1]
    rows = table.find_all('tr')
    for row in rows:
        cells = row.find_all('td')
        if len(cells) == 0:
            continue
        str_cells = [x.text.strip() for x in cells]

        moon = {}

        moon['name'] = clear_name(str_cells[1])
        diameter_value, dimensions_value = get_diameter_and_dimensions(str_cells[5])
        moon['diameter'] = diameter_value
        if len(dimensions_value):
            moon['dimensions'] = dimensions_value
        moon['mass'] = get_mass_value(str_cells[6], 'e16')
        moon['group'] = str_cells[14]
        moon['parent'] = 'Jupiter'

        result.append(moon)

    return result

    with open('jupiter_moons.json', 'w') as file:
        json.dump(result, file, indent=4)

def process_saturn_moons():
    url = "https://en.wikipedia.org/wiki/Moons_of_Saturn"

    response = requests.get(url)

    # Parse the HTML content
    soup = BeautifulSoup(response.text, 'html.parser')

    # Find the table you want to parse
    tables = soup.find_all('table', {'class': 'wikitable'})

    #for table in tables:
    result = []
    table = tables[2]
    rows = table.find_all('tr')
    for row in rows:
        cells = row.find_all('td')
        if len(cells) == 0:
            continue
        str_cells = [x.text.strip() for x in cells]
        print(str_cells)
        print(str_cells[1], str_cells[5], str_cells[6])

        moon = {}
        name = str_cells[1]

        if 'moonlets' in name:
            continue

        moon['name'] = clear_name(name)
        diameter_value, dimensions_value = get_diameter_and_dimensions(str_cells[5])
        moon['diameter'] = diameter_value
        if len(dimensions_value):
            moon['dimensions'] = dimensions_value
        moon['mass'] = get_mass_value(str_cells[6], 'e15')

        if '♠' in name:
            moon['group'] = 'Major'
        elif '†' in name:
            moon['group'] = 'Prograde'
        elif '♣' in name:
            moon['group'] = 'Gallic'
        elif '‡' in name:
            moon['group'] = 'Norse'
        elif '§' in name:
            moon['group'] = 'Outlier'

        moon['parent'] = 'Saturn'

        result.append(moon)
    
    return result

def process_uranus_moons():
    url = "https://en.wikipedia.org/wiki/Moons_of_Uranus"
    response = requests.get(url)
    soup = BeautifulSoup(response.text, 'html.parser')
    tables = soup.find_all('table', {'class': 'wikitable'})

    result = []
    table = tables[1]
    rows = table.find_all('tr')
    for row in rows:
        cells = row.find_all('td')
        if len(cells) == 0:
            continue
        str_cells = [x.text.strip() for x in cells]
        print(str_cells[1], str_cells[5], str_cells[6])

        moon = {}
        name = str_cells[1]
        moon['name'] = clear_name(name)
        diameter_value, dimensions_value = get_diameter_and_dimensions(str_cells[5])
        moon['diameter'] = diameter_value
        if len(dimensions_value):
            moon['dimensions'] = dimensions_value
        moon['mass'] = get_mass_value(str_cells[6], 'e16')

        if '♠' in name:
            moon['group'] = 'Major'
        elif '†' in name:
            moon['group'] = 'Prograde'
        elif '‡' in name:
            moon['group'] = 'Retrograde'
        elif '♦' in name:
            moon['group'] = 'Caliban'

        moon['parent'] = 'Uranus'

        result.append(moon)
    
    return result

def process_neptune_moons():
    url = "https://en.wikipedia.org/wiki/Moons_of_Neptune"
    response = requests.get(url)
    soup = BeautifulSoup(response.text, 'html.parser')
    tables = soup.find_all('table', {'class': 'wikitable'})

    result = []
    table = tables[1]
    rows = table.find_all('tr')
    for row in rows:
        cells = row.find_all('td')
        if len(cells) == 0:
            continue
        str_cells = [x.text.strip() for x in cells]
        print(str_cells[1], str_cells[5], str_cells[6])

        moon = {}
        name = str_cells[1]
        moon['name'] = clear_name(name)
        diameter_value, dimensions_value = get_diameter_and_dimensions(str_cells[5])
        moon['diameter'] = diameter_value
        if len(dimensions_value):
            moon['dimensions'] = dimensions_value
        moon['mass'] = get_mass_value(str_cells[6], 'e16')

        if '♠' in name:
            moon['group'] = 'Triton'
        elif '†' in name:
            moon['group'] = 'Nereid'
        elif '‡' in name:
            moon['group'] = 'Halimede'
        elif '♦' in name:
            moon['group'] = 'Sao'
        elif '♥' in name:
            moon['group'] = 'Neso'

        moon['parent'] = 'Uranus'

        result.append(moon)
    
    return result

result = process_jupiter_moons()
result.extend(process_saturn_moons())
result.extend(process_uranus_moons())
result.extend(process_neptune_moons())

with open('moons.json', 'w') as file:
    json.dump(result, file, indent=4)
