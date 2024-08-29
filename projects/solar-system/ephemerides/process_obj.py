import numpy as np

def load_obj(file_path):
    vertices = []
    faces = []
    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith('v '):  # Only consider lines defining vertices
                parts = line.split()
                vertex = list(map(float, parts[1:4]))
                vertices.append(vertex)
            elif line.startswith('f '):  # Consider lines defining faces
                parts = line.split()
                face = [int(idx.split('/')[0]) for idx in parts[1:]]
                faces.append(face)
    return np.array(vertices), faces

def save_minified_obj(file_path, vertices, faces):
    with open(file_path, 'w') as new_file:
        for vertex in vertices:
            # Round vertex coordinates to 6 decimal places
            vertex_str = ' '.join(f"{coord:.6f}" for coord in vertex)
            new_file.write(f"v {vertex_str}\n")
        for face in faces:
            face_str = ' '.join(map(str, face))
            new_file.write(f"f {face_str}\n")

def normalize_vertices(vertices):
    # Find min and max in each dimension
    min_vals = np.min(vertices, axis=0)
    max_vals = np.max(vertices, axis=0)
    
    # Calculate the center and the range
    center = (min_vals + max_vals) / 2
    range_vals = max_vals - min_vals
    
    # Normalize vertices
    max_range = np.max(range_vals)
    normalized_vertices = (vertices - center) / (max_range / 2)
    
    return normalized_vertices

def process_obj(file_path, output_path):
    vertices, faces = load_obj(file_path)
    normalized_vertices = normalize_vertices(vertices)
    save_minified_obj(output_path, normalized_vertices.tolist(), faces)

# Example usage
input_file = 'Astraea.obj'  # Replace with your input OBJ file path
output_file = 'Astraea2.obj'  # Replace with your desired output OBJ file path
process_obj(input_file, output_file)
