import json, csv
from astroquery.jplsbdb import SBDB

def read_input_file(filename):
    spkids, names = [], []
    with open(filename) as file:
        data = csv.reader(file, delimiter=',', quotechar='"')
        for row in data:
            spkids.append(row[0].strip())
            names.append(row[1].strip())
    return spkids[1:], names[1:]

def print_body(data):
    if 'shortname' in data['object']:
        short_name = data['object']['shortname']
    if 'fullname' in data['object']:
        full_name = data['object']['fullname']
    if 'diameter' in data['phys_par']:
        diameter = data['phys_par']['diameter'] # km
    if 'GM' in data['phys_par']:
        GM = data['phys_par']['GM'] # km3/s2
    if 'density_sig' in data['phys_par']:
        density = data['phys_par']['density_sig'] # g/cm3
    eccentricity = data['orbit']['elements']['e']
    inclination = data['orbit']['elements']['i']
    ascending_node_longitude = data['orbit']['elements']['om']
    argument_of_perihelion = data['orbit']['elements']['w']
    mean_anomaly = data['orbit']['elements']['ma']
    semi_major_axis = data['orbit']['elements']['a']

    if 'shortname' in data['object']:
        print(f'short name: {short_name}')
    if 'fullname' in data['object']:
        print(f'full name: f{full_name}')
    if 'diameter' in data['phys_par']:
        print(f'diameter: {diameter}')
    if 'GM' in data['phys_par']:
        print(f'GM: {GM}')
    if 'density_sig' in data['phys_par']:
        print(f'density: {density}')
    print(f'eccentricity: {eccentricity}')
    print(f'inclination: {inclination}')
    print(f'ascending_node_longitude: {ascending_node_longitude}')
    print(f'argument_of_perihelion: {argument_of_perihelion}')
    print(f'mean_anomaly: {mean_anomaly}')
    print(f'semi_major_axis: {semi_major_axis}\n')

def get_body_from_sbdb(spkid):
    data = SBDB.query(spkid, phys=True)

    print_body(data)

    body = {}
    body['Spkid'] = spkid
    if 'shortname' in data['object']:
        body['BodyName'] = data['object']['shortname']
    else:
        body['BodyName'] = data['object']['fullname']
    body['AttractorName'] = 'Solar system barycenter'
    body['AttractorMass'] = 1.9884999721201209e30
    body['EC'] = data['orbit']['elements']['e'] # eccentricity
    body['IN'] = data['orbit']['elements']['i'].value # inclination [deg]
    body['OM'] = data['orbit']['elements']['om'].value # ascending node longitude [deg]
    body['W'] = data['orbit']['elements']['w'].value # argument of perihelion [deg]
    body['MA'] = data['orbit']['elements']['ma'].value # mean anomaly [deg]
    body['A'] = data['orbit']['elements']['a'].value # semi-major axis [AU]
    if 'diameter' in data['phys_par']:
        if type(data['phys_par']['diameter']) == str:
            body['Diameter'] = float(data['phys_par']['diameter']) # diameter [km]
        else:
            body['Diameter'] = data['phys_par']['diameter'].value # diameter [km]
    if 'GM' in data['phys_par']:
        body['GM'] = data['phys_par']['GM'].value # GM km3/s2
    if 'density' in data['phys_par']:
        if type(data['phys_par']['density']) == str:
            body['Density'] = float(data['phys_par']['density']) # density g/cm3
        else:
            body['Density'] = data['phys_par']['density'].value # density g/cm3
    return body

def process_bodies_from_csv(csvfile):
    spkids, names = read_input_file(csvfile)

    result = {}
    result['OrbitsData'] = []
    for spkid in spkids:
        result['OrbitsData'].append(get_body_from_sbdb(spkid))

    with open('..\data\small-bodies-sbdb.json', 'w') as file:
        json.dump(result, file)

process_bodies_from_csv('sbdb_query_diameter_100km.csv')