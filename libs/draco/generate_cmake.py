import os

def collect_files_recursive(directory):
    collected_files = []

    for root, dirs, files in os.walk(directory):
        for file in files:
            if not file.endswith('.py') and not file.endswith('.txt'):
                # Construct the relative file path with forward slashes
                relative_path = os.path.relpath(os.path.join(root, file), directory)
                normalized_path = relative_path.replace(os.sep, '/')
                collected_files.append(normalized_path)

    return collected_files

# Input directory
input_directory = "."

# Call the function and store the result
file_paths = collect_files_recursive(input_directory)


with open('CMakeLists.txt', 'w') as file:
    file.write('# version 1.5.7\n')
    file.write('fips_begin_lib(draco)\n')
    file.write('fips_files(\n')
    for path in file_paths:
        file.write('    ' + path + '\n')
    file.write(')\n')
    file.write('fips_end_lib(draco)\n')

#for path in file_paths:
#    print(path)