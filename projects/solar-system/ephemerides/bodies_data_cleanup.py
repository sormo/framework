import json

def process_major_bodies(file_path):
    with open(file_path, 'r') as file:
        data = json.load(file)

    for body in data:
        group = body['group']
        body_type = 'Unknown'

        if group == 'Barycenter' or group == 'Start' or group == 'Planet' or group == 'Moon' or group == 'Dwarf Planet':
            body_type = group
            group = ''
        elif group == 'Lagrange':
            body_type = 'Lagrange Point'
            group = ''
        else:
            body_type = 'Moon'

        body['group'] = group
        body['type'] = body_type

    with open(file_path, 'w') as file:
        json.dump(data, file, indent=4)

def process_small_bodies(file_path):
    with open(file_path, 'r') as file:
        data = json.load(file)

    for body in data:
        group = body['group']
        body_type = 'Minor Planet'
        if 'Comet' in group:
            body_type = 'Comet'

        if group == 'Halley-type Comet*':
            group = 'Halley-type Comet'

        body['group'] = group
        body['type'] = body_type

    with open(file_path, 'w') as file:
        json.dump(data, file, indent=4)

def print_groups(file_path):
    with open(file_path, 'r') as file:
        data = json.load(file)
    all_groups = []
    for body in data:
        group = body['group']
        if group not in all_groups:
            all_groups.append(group)
    for group in all_groups:
        print(f'"{group}": "#ffffff",') 

print('major-bodies:')
print_groups('../data/major-bodies.json')
print('')

print('small-bodies:')
print_groups('../data/small-bodies-sbdb-10km.json')
print('')

#process_major_bodies('../data/major-bodies.json')
#process_small_bodies('../data/small-bodies-sbdb-10km.json')
#process_small_bodies('../data/small-bodies-sbdb-50km.json')
#process_small_bodies('../data/small-bodies-sbdb-100km.json')
