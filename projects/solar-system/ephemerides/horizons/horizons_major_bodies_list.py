import json

lines = []

with open('horizons-major-bodies-list.txt', 'r') as file:
    for line in file:
        lines.append(line)

# skip first two lines - columns names and separator row
lines = lines[2:]

result = []
barycenters = {}

for line in lines:
    id = int(line[:10].strip())
    name = line[11:46].strip()
    designation = line[47:58].strip()
    other = line[59:79].strip()

    print(id, name, designation, other)

    body = {}
    body['name'] = name if len(name) else designation
    body['id'] = id

    if 'Barycenter' in name:
        barycenters[name.split(' ')[0]] = id
        body['parent'] = 0
    elif len(other) == 0:
        body['parent'] = 0
    else:
        if other[0] == 'E':
            body['parent'] = barycenters['Earth-Moon']
        elif other[0] == 'M':
            body['parent'] = barycenters['Mars']
        elif other[0] == 'J':
            body['parent'] = barycenters['Jupiter']
        elif other[0] == 'S':
            body['parent'] = barycenters['Saturn']
        elif other[0] == 'U':
            body['parent'] = barycenters['Uranus']
        elif other[0] == 'N':
            body['parent'] = barycenters['Neptune']
        elif other[0] == 'P':
            body['parent'] = barycenters['Pluto']

    result.append(body)

with open('horizons-major-bodies-list.json', 'w') as file:
    json.dump(result, file, indent=4)