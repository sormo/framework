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

    return result

def get_orbital_elements(elems):
    # print('eccentricity:', elems['e'].value[0])
    # print('semi-major axis:', elems['a'].value[0], 'AU')
    # print('inclination', elems['incl'].value[0], 'deg')
    # print('longitude of ascending node', elems['Omega'].value[0], 'deg')
    # print('argunment of perifocus', elems['w'].value[0], 'deg')
    # print('mean anomaly', elems['M'].value[0], 'deg')

    result = {}
    result['EC'] = elems['e'].value[0]
    result['IN'] = elems['incl'].value[0]
    result['OM'] = elems['Omega'].value[0]
    result['W'] = elems['w'].value[0]
    result['MA'] = elems['M'].value[0]
    result['A'] = elems['a'].value[0]

    return result

def get_horizons_body(body_id, parent_id, time):
    target_location = f'500@{parent_id}'

    query = Horizons(id=body_id, location=target_location, epochs=time)

    id = query.id

    raw_data = str(query.elements_async().content)
    if body_id == 0:
        orbit_data = get_orbital_elements_empty()
    else:
        try:
            #elements = query.elements(cache=False)
            elements = query.elements(cache=True)
        except Exception as e:
            print('Exception: ', e)
            return {}
        orbit_data = get_orbital_elements(elements)

    phys_data = parse_phys_data(raw_data)

    result = orbit_data
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

def process_missing_bodies():
    data = ['Ersa', 'S/2018 J 2', 'Pandia', 'S/2011 J 3', 'S/2018 J 4', 'Valetudo', 'S/2021 J 3', 'S/2016 J 1', 'S/2021 J 2', 'S/2017 J 3', 'S/2021 J 1', 'S/2017 J 7', 'S/2022 J 3', 'S/2017 J 9', 'S/2016 J 3', 'S/2022 J 1', 'S/2017 J 8', 'S/2021 J 6', 'S/2003 J 24', 'S/2017 J 2', 'S/2022 J 2', 'S/2021 J 4', 'S/2016 J 4', 'S/2017 J 5', 'S/2017 J 6', 'S/2018 J 3', 'S/2021 J 5', 'S/2017 J 1', 'S/2009 S 1', 'S/2019 S 1', 'S/2005 S 4', 'S/2020 S 1', 'S/2006 S 20', 'S/2006 S 9', 'S/2007 S 5', 'S/2007 S 7', 'S/2004 S 37', 'S/2004 S 47', 'S/2004 S 40', 'S/2019 S 2', 'S/2007 S 8', 'S/2004 S 29', 'S/2019 S 3', 'S/2020 S 7', 'S/2004 S 31', 'S/2019 S 14', 'S/2020 S 2', 'S/2019 S 4', 'S/2020 S 3', 'S/2004 S 41', 'S/2020 S 4', 'S/2004 S 42', 'S/2020 S 5', 'S/2007 S 6', 'S/2004 S 43', 'S/2006 S 10', 'S/2019 S 5', 'Gridr', 'S/2004 S 44', 'S/2006 S 12', 'S/2004 S 45', 'S/2006 S 11', 'Eggther', 'S/2006 S 13', 'S/2019 S 6', 'S/2007 S 9', 'S/2019 S 7', 'S/2019 S 8', 'S/2019 S 9', 'S/2004 S 46', 'Angrboda', 'S/2019 S 11', 'Beli', 'S/2019 S 10', 'S/2019 S 12', 'Gerd', 'S/2019 S 13', 'S/2006 S 14', 'Gunnlod', 'S/2019 S 15', 'S/2020 S 6', 'S/2005 S 5', 'Skrymir', 'S/2006 S 16', 'S/2006 S 15', 'S/2004 S 28', 'S/2020 S 8', 'Alvaldi', 'S/2004 S 48', 'Geirrod', 'S/2004 S 50', 'S/2006 S 17', 'S/2004 S 49', 'S/2019 S 17', 'S/2006 S 18', 'S/2019 S 19', 'S/2004 S 21', 'S/2019 S 18', 'S/2004 S 39', 'S/2019 S 16', 'S/2004 S 53', 'S/2004 S 24', 'S/2004 S 36', 'Thiazzi', 'S/2019 S 20', 'S/2006 S 19', 'S/2004 S 34', 'S/2004 S 51', 'S/2020 S 10', 'S/2020 S 9', 'S/2004 S 26', 'S/2019 S 21', 'S/2004 S 52', 'S/2023 U 1', 'S/2002 N 5', 'S/2021 N 1']

    jupiter_moons= ['Ersa', 'Pandia', 'Valetudo']

    def normalize_name(name):
        if '/' in name:
            name = name[2:6]+name[7] + name[9:]
        return name

    result = []
    for body in data:
        if '/' in body:
            parent = 5 if 'J' in body else 6
        elif body in jupiter_moons:
            parent = 5
        else:
            parent = 6

        body_name = normalize_name(body)

        print(f'Processing {body_name}')

        horizons_body = get_horizons_body(body_name, parent, Time(['2018-01-01']))
        
        horizons_body['name'] = body_name
        horizons_body['parent'] = parent
        #horizons_body['horizons_id'] = body['id']
        horizons_body['time'] = '2018-01-01'
        
        result.append(horizons_body)

    with open('missing-bodies.json', 'w') as file:
        json.dump(result, file, indent=4)


#process_horizon_major_bodies('2018-01-01')

#jupiter = get_horizons_body(599, 0, Time(['2018-01-01']))
#himalaia = get_horizons_body(506, 5, Time(['2018-01-01']))
#telesto = get_horizons_body(613, 6, Time(['2018-01-01']))
#philophrosyne = get_horizons_body(558, 5, Time(['2018-01-01']))
# barycenter = get_horizons_body(53092511, 0, Time(['2018-01-01']))
# print(barycenter)
# primary = get_horizons_body(953092511, 53092511, Time(['2018-01-01']))
# print(primary)
# secondary = get_horizons_body(153092511, 53092511, Time(['2018-01-01']))
# print(secondary)

quaoar = get_horizons_body(599, 5, Time(['2018-01-01']))
print(quaoar)

