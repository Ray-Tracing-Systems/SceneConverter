import os
import subprocess


def list_files_in_dir(walk_dir):
    files_list = []

    for root, subdirs, files in os.walk(walk_dir):
        for filename in files:
            root_ = root.replace("""\\""", """/""")
            files_list.append(f'{root_}/{filename}')

    return files_list


def list_files_with_ext_in_dir(walk_dir, ext: str):
    files_list = []

    for root, subdirs, files in os.walk(walk_dir):
        for filename in files:
            if filename.endswith(ext):
                root_ = root.replace("""\\""", """/""")
                files_list.append(f'{root_}/{filename}')

    return files_list


def list_subdirs_in_dir(walk_dir):
    return [f.name for f in os.scandir(walk_dir) if f.is_dir()]


def my_makedirs(path):
    try:
        os.makedirs(path, mode=777)
    except FileExistsError as error:
        pass
    except OSError as error:
        print(error)


if __name__ == '__main__':
    converter_path = 'bin/scene-converter.exe'
    model_dirs = ['D:/3d models/glTF-Sample-Models/2.0', 'D:/3d models/3ddd']
    output_dir = 'D:/3d models/converted'
    collapse_meshes = False
    convert_gltf_materials = True

    my_makedirs(output_dir)

    gltf = []
    obj = []
    for m in model_dirs:
        gltf.extend(list_files_with_ext_in_dir(m, '.gltf'))
        obj.extend(list_files_with_ext_in_dir(m, '.obj'))

    print(f'Total gltf files: {len(gltf)}\n')
    print(f'Total obj files: {len(obj)}\n')

    print('Starting .obj files conversion\n')
    i = 1
    for o in obj:
        _, filename = os.path.split(o)
        name, ext = os.path.splitext(filename)
        cmd = [converter_path, o, f'{output_dir}/{name}']
        if collapse_meshes:
            cmd.append('--collapse')
        subprocess.run(cmd)
        print(f'Converted {i} out of {len(obj)}\n')
        i += 1

    print('Starting .gltf files conversion\n')
    i = 1
    for o in gltf:
        _, filename = os.path.split(o)
        name, ext = os.path.splitext(filename)
        cmd = [converter_path, o, f'{output_dir}/{name}']
        if collapse_meshes:
            cmd.append('--collapse')
        if convert_gltf_materials:
            cmd.append('--convert-materials')
        subprocess.run(cmd)
        print(f'Converted {i} out of {len(gltf)}\n')
        i += 1
