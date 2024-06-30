import json, csv
from astroquery.jplsbdb import SBDB
from datetime import datetime, timedelta

def julian_to_utc(julian_date):
    # Julian date for January 1, 2000 at 12:00 UTC
    julian_start = datetime(2000, 1, 1, 12)
    # Difference in Julian days from the reference date
    delta_days = julian_date - 2451545.0
    # Convert to UTC date
    utc_date = julian_start + timedelta(days=delta_days)
    return utc_date

def format_date(utc_date):
    return utc_date.strftime("%Y-%m-%d")

def read_input_file(filename):
    spkids, names = [], []
    with open(filename) as file:
        data = csv.reader(file, delimiter=',', quotechar='"')
        for row in data:
            spkids.append(row[0].strip())
            names.append(row[1].strip())
    return spkids[1:], names[1:]

def get_type(kind):
    if kind == 'cn' or kind == 'cu':
        return 'Comet'
    return 'Minor Planet'


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
    epoch = data['orbit']['epoch']
    kind = data['object']['kind']

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
    print(f'semi_major_axis: {semi_major_axis}')
    print(f'type: {get_type(kind)}')
    print(f'time: {format_date(julian_to_utc(epoch.value))}\n')

def get_body_from_sbdb(spkid):
    data = SBDB.query(spkid, phys=True)

    print_body(data)

    if data['orbit']['two_body']:
        print('found two-body!')

    body = {}
    body['Spkid'] = spkid
    if 'shortname' in data['object']:
        body['name'] = data['object']['shortname']
    else:
        body['name'] = data['object']['fullname']
    body['parent_name'] = 'Solar-System Barycenter'
    body['period'] = data['orbit']['elements']['per'].value # orbit period [days]
    body['EC'] = data['orbit']['elements']['e'] # eccentricity
    body['IN'] = data['orbit']['elements']['i'].value # inclination [deg]
    body['OM'] = data['orbit']['elements']['om'].value # ascending node longitude [deg]
    body['W'] = data['orbit']['elements']['w'].value # argument of perihelion [deg]
    body['MA'] = data['orbit']['elements']['ma'].value # mean anomaly [deg]
    body['A'] = data['orbit']['elements']['a'].value # semi-major axis [AU]
    body['type'] = get_type(data['object']['kind'])
    body['time'] = format_date(julian_to_utc(data['orbit']['epoch'].value))
    if 'diameter' in data['phys_par']:
        if type(data['phys_par']['diameter']) == str:
            body['radius'] = float(data['phys_par']['diameter']) / 2.0 # diameter [km]
        else:
            body['radius'] = data['phys_par']['diameter'].value  / 2.0 # diameter [km]
    if 'GM' in data['phys_par']:
        body['gm'] = data['phys_par']['GM'].value # GM km3/s2
    if 'density' in data['phys_par']:
        if type(data['phys_par']['density']) == str:
            body['density'] = float(data['phys_par']['density']) # density g/cm3
        else:
            body['density'] = data['phys_par']['density'].value # density g/cm3
    if 'rot_per' in data['phys_par']:
        if type(data['phys_par']['rot_per']) == str:
            body['rotation_period'] = float(data['phys_par']['rot_per']) # rotation period [hours]
        else:
            body['rotation_period'] = data['phys_par']['rot_per'].value # rotation period [hours]
    
    if 'orbit_class' in data['object']:
        body['group'] = data['object']['orbit_class']['name']

    return body

def process_bodies_from_csv(csvfile, output_jsonfile):
    spkids, names = read_input_file(csvfile)

    result = []
    for spkid in spkids:
        result.append(get_body_from_sbdb(spkid))

    with open(output_jsonfile, 'w') as file:
        json.dump(result, file, indent=4)

#test = get_body_from_sbdb('20000003')

process_bodies_from_csv('sbdb_query_diameter_10km.csv', 'small-bodies-sbdb-10km.json')
