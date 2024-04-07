from astroquery.jplhorizons import Horizons
from astropy.time import Time
import re, json

with open('major-bodies-final.json', 'r') as file:
    data = json.load(file)

def get_horizon_id(name):
    target_location = f'500@0'
    query = Horizons(id=name, location=target_location, epochs=Time(['2018-01-01']))

    return query.id

name_body_map = {}
id_name_map = {}
childs = {}
total_masses = {}

def compute_total_mass(name):
    body = name_body_map[name]
    mass = body['mass']
    
    if name not in childs:
        return mass
    
    for child in childs[name]:
        mass += compute_total_mass(child)
    
    return mass

for d in data:
    if 'parent' not in d:
        print('Error, no parent for', d['name'])
    if 'horizons_id' in d:
        id_name_map[d['horizons_id']] = d['name']
    name_body_map[d['name']] = d

for d in data:
    if d['parent'] == -1:
        continue

    parent = id_name_map[d['parent']]
    if parent not in childs:
        childs[parent] = []
    childs[parent].append(d['name'])

for d in data:
    total_masses[d['name']] = compute_total_mass(d['name'])

for d in data:
    if d['parent'] == -1:
        continue

    parent = id_name_map[d['parent']]
    parent_mass = total_masses[parent]
    d['attractor_name'] = parent
    # d['attractor_mass'] = parent_mass - d['mass']
    d['attractor_mass'] = parent_mass

with open('major-bodies-final-attractors.json', 'w') as file:
    data = json.dump(data, file, indent=4)
