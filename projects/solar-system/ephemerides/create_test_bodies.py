import json

#bodies_to_add = [ 0, 2, 5, 7, 90377 ]
bodies_to_add = [ 0, 2, 5, 7 ]

data = {}

with open('../data/major-bodies.json', 'r') as file:
    data = json.load(file)

test_data = []

for body in data:
    if 'horizons_id' in body and body['horizons_id'] in bodies_to_add:
        test_data.append(body)

with open('../data/test-bodies.json', 'w') as file:
    json.dump(test_data, file, indent=4)