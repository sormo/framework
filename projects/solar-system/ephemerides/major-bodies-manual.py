import re, json

with open('major-bodies-manual-4.json', 'r') as file:
    data = json.load(file)

with open('wikipedia/moons.json', 'r') as file:
    moons = json.load(file)

def normalize_name(name):
    if '/' in name:
        name = name[2:6]+name[7] + name[9:]
    return name.lower()

def find_moon(name, data):
    for m in data:
        if normalize_name(m['name']) == normalize_name(name):
            return m

def copy_data_from(body_from, body_to):
    if 'mass' in body_from and 'mass' not in body_to:
        body_to['mass'] = body_from['mass']
    if 'diameter' in body_from and 'diameter' not in body_to:
        body_to['radius'] = body_from['diameter'] / 2.0
    if 'dimensions' in body_from and 'dimensions' not in body_to:
        body_to['dimensions'] = body_from['dimensions']
    if 'group' in body_from and 'group' not in body_to:
        body_to['group'] = body_from['group']

missing_in_data = []
missing_in_moons = []

for body in data:
    m = find_moon(body['name'], moons)
    if m is None:
        missing_in_moons.append(body['name'])
    else:
        copy_data_from(m, body)

for body in moons:
    m = find_moon(body['name'], data)
    if m is None:
        missing_in_data.append(body['name'])

print('missing in moons:')
print(missing_in_moons)

print('missing in data:')
print(missing_in_data)

with open('major-bodies-manual-5.json', 'w') as file:
    data = json.dump(data, file, indent=4)
