from astroquery.jplhorizons import Horizons
from astropy.time import Time
import re, json

# density : g/cm^3
# gm : km^3/s^2
# radius : km
# mass : kg ?
def parse_phys_data(raw_data):
    result = {}

    rmass = re.match(r'.*Mass x[ ]?10\^([0-9]*\b) \(([\w]*)\)[ ]*=[ ]*(\b[0-9.]*\b).*', raw_data)
    if not rmass:
        rmass = re.match(r'.*Mass[,]?[ ]*10\^([0-9]*)[ ]*(\b[a-z]*\b)[ ]*=[ ]*[~]?([0-9\.]*)\b.*', raw_data)
    if not rmass:
        rmass = re.match(r'.*Mass[,]?[ ]*x[ ]?10\^([0-9]*\b)[ ]*[\(]?([\w]*)[\)]?[ ]*=[ ]*(\b[0-9.]*\b).*', raw_data)
    if rmass:
        mass = rmass.group(3) + 'e' + rmass.group(1) + ' ' + rmass.group(2)
        result['mass'] = mass
    
    if not rmass:
        rmass = re.match(r'.*Mass[ ]*\(([\w\^ ]*)\)[ ]*=[ ]*([0-9.]*)[ ]*\(([\w\^ -]*)\).*', raw_data)
        if rmass:
            mass = rmass.group(2) + '*' + rmass.group(3) + '*' + rmass.group(1)

    if not rmass:
        rmass = re.match(r'.*Mass[,]?[ ]*\(10\^([0-9]*) g\)[ ]*[ ]*=[ ]*[~]?([0-9\.]*)\b.*', raw_data)
        if rmass:
            mass = rmass.group(2) + 'e' + str(int(rmass.group(1)) - 3)

    rdensity = re.match(r'.*Density[ ]*\(([\w \^\\\-0-9]*)\)[ ]*=[ ]*([0-9.]*).*', raw_data)
    if not rdensity:
        rdensity = re.match(r'.*Mean density[,]? (.*?)[ ]*=[ ]*([0-9.]*\b).*', raw_data)
    if not rdensity:
        rdensity = re.match(r'.*Density[,]?[ ]*[\(]?(.*?)[\)]?[ ]*=[ ]*([0-9.]*\b).*', raw_data)
    if rdensity:
        value = rdensity.group(2).strip()
        if len(value) != 0:
            density = rdensity.group(2)
            result['density'] = density
    rradius = re.match(r'.*Vol\. Mean Radius \((.*?)\)[ ]*=[ ]*([0-9.]*).*', raw_data)
    if not rradius:
        rradius = re.match(r'.*Vol\. [M|m]ean [R|r]adius[,]? [\(]?([\w]*)[\)]?[ ]*=[ ]*([0-9.]*).*', raw_data)
    if not rradius:
        rradius = re.match(r'.*Radius[ ]*[\(]?(\w*)[\)]?[ ]*=[ ]*([0-9.x ]*)\b.*', raw_data)
    if not rradius:
        rradius = re.match(r'.*Radius \(\w*\)[ ]*=[ ]*([0-9.]*)[ ]*([\w]*).*', raw_data)
    if not rradius:
        rradius = re.match(r'.*[M|m]ean [R|r]adius[ ]*[\(]?([\w]*)[\)]?[ ]*=[ ]*([0-9.]*)[ ]*.*', raw_data)
    if not rradius:
        rradius = re.match(r'.*RAD[,]?[ ]*=[ ]*([\w.]*).*', raw_data)
    if rradius:
        if len(rradius.groups()) == 2:
            radius = rradius.group(2) + ' ' + rradius.group(1)
        else:
            radius = rradius.group(1)
        result['radius'] = radius
    rgm = re.match(r'.*GM \(([\w\^\/]*)\)[ ]*= ([0-9.]*\b)[ ]*.*', raw_data)
    if not rgm:
        rgm = re.match(r'.*GM[,]? [\(]?([\w\^\/]*)[\)]?[ ]*= ([0-9.]*\b)[ ]*.*', raw_data)
    if not rgm:
        rgm = re.match(r'.*GM[,]?[ ]*=[ ]*([\w.]*).*', raw_data)
    if not rgm:
        rgm = re.match(r'.*GM[,]?[ ]*[\(]?([\w\^\/]*)[\)]?[ ]*= ([0-9.]*\b)[ ]*.*', raw_data)
    if rgm:
        if len(rgm.groups()) == 2:
            gm = rgm.group(2) + ' ' + rgm.group(1)
        else:
            gm = rgm.group(1)
        result['gm'] = gm

    return result

def get_orbital_elements_empty():
    result = {}
    result['EC'] = 0.0
    result['IN'] = 0.0
    result['OM'] = 0.0
    result['W'] = 0.0
    result['MA'] = 0.0
    result['A'] = 0.0
    result['period'] = 0.0
    result['position'] = [0.0,0.0,0.0]
    result['velocity'] = [0.0,0.0,0.0]

    return result

def get_orbital_elements(elems, vectors):
    print('eccentricity:', elems['e'].value[0])
    print('semi-major axis:', elems['a'].value[0], 'AU')
    print('inclination', elems['incl'].value[0], 'deg')
    print('longitude of ascending node', elems['Omega'].value[0], 'deg')
    print('argument of perifocus', elems['w'].value[0], 'deg')
    print('mean anomaly', elems['M'].value[0], 'deg')
    print('period', elems['P'].value[0], 'days')
    if vectors:
        print('position', vectors['x'].value[0], vectors['y'].value[0], vectors['z'].value[0], 'AU')
        print('velocity', vectors['vx'].value[0], vectors['vy'].value[0], vectors['vz'].value[0], 'AU/d')

    result = {}
    result['EC'] = elems['e'].value[0]
    result['IN'] = elems['incl'].value[0]
    result['OM'] = elems['Omega'].value[0]
    result['W'] = elems['w'].value[0]
    result['MA'] = elems['M'].value[0]
    result['A'] = elems['a'].value[0]
    result['period'] = elems['P'].value[0]
    if vectors:
        result['position'] = [vectors['x'].value[0], vectors['y'].value[0], vectors['z'].value[0]]
        result['velocity'] = [vectors['vx'].value[0], vectors['vy'].value[0], vectors['vz'].value[0]]

    return result

def get_horizons_body(body_id, parent_id, time, with_phys_data=True):
    target_location = f'500@{parent_id}'

    query = Horizons(id=body_id, location=target_location, epochs=time)

    id = query.id

    if body_id == 0:
        orbit_data = get_orbital_elements_empty()
    else:
        try:
            elements = query.elements(cache=True)
            vectors = query.vectors(cache=True)
        except Exception as e:
            print('Exception: ', e)
            return {}
        orbit_data = get_orbital_elements(elements, vectors)

    result = orbit_data

    if with_phys_data:
        raw_data = str(query.elements_async().content)
        phys_data = parse_phys_data(raw_data)
        result.update(phys_data)

    return result

def process_horizon_major_bodies(time):
    with open('horizons-major-bodies-list.json', 'r') as file:
        major_bodies = json.load(file)

    result = []
    for body in major_bodies:
        body_name = body['name']

        print(f'Processing {body_name}')

        horizons_body = get_horizons_body(body['id'], body['parent'], Time([time]))
        
        horizons_body['name'] = body_name
        horizons_body['parent'] = body['parent']
        horizons_body['horizons_id'] = body['id']
        horizons_body['time'] = time

        result.append(horizons_body)

    with open('horizons-major-bodies.json', 'w') as file:
        json.dump(result, file, indent=4)

def update_horizons_major_bodies(file_in, file_out, time):
    with open(file_in, 'r') as file:
        bodies = json.load(file)
    
    for body in bodies:
        id = body['horizons_id'] if 'horizons_id' in body else body['name']
        parent = body['parent']

        if parent == -1:
            continue

        print('Processing', body['name'])

        horizons_body = get_horizons_body(id, parent, Time([time]), False)
        for k,v in horizons_body.items():
            body[k] = v
        body['time'] = time
    
    with open(file_out, 'w') as file:
        json.dump(bodies, file, indent=4)

update_horizons_major_bodies('major-bodies.json', 'major-bodies-updated.json', '2024-03-31')

#process_horizon_major_bodies('2018-01-01')
# test = get_horizons_body(5, 0, Time(['2018-01-01']))

# for key, value in test.items():
#     print(f'        "{key}": {value},')
