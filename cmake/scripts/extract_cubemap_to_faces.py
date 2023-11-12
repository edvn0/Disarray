from io import BytesIO
import re
from typing import List
from urllib.parse import ParseResult, urlparse
from PIL import Image
from argparse import ArgumentParser, Namespace
from pathlib import Path
import subprocess

import requests


def extract_texture_map_faces(image_or_path: str | Image.Image, output_base_name: str = 'output_face'):
    if isinstance(image_or_path, str):
        try:
            texture_map = Image.open(Path(image_or_path))
        except FileNotFoundError:
            print(f"Could not open {str(image_or_path)}")
            return []
    else:
        texture_map: Image.Image = image_or_path

    width, height = texture_map.size
    print(f"Image width and height is {width} by {height}.")

    cell_width = width // 4
    cell_height = height // 3

    if width == height:
        cell_width = cell_height = width // 4

    faces = {
        'left': (0, cell_height, 				cell_width, 	cell_height * 2),
        'front': (cell_width, cell_height, 		cell_width * 2, cell_height * 2),
        'right': (cell_width * 2, cell_height, 	cell_width * 3, cell_height * 2),
        'back': (cell_width * 3, cell_height, 	cell_width * 4, cell_height * 2),
        'top': (cell_width, 0, cell_width * 2, cell_height),
        'bottom': (cell_width, cell_height * 2, cell_width * 2, cell_height * 3)
    }

    names: List[str] = []
    for face_name, coordinates in faces.items():
        face_image = texture_map.crop(coordinates)
        output_filename = f'{output_base_name}_{face_name}.png'
        face_image.save(output_filename)
        names.append(output_filename)
        print(f'Saved {output_filename}')
    return names


def convert_to_ktx(filepaths: List[str]):
	import shutil
	path = shutil.which("ktx")

	if path is None:
		print("no executable found for command 'ktx'")
		return

	if len(filepaths) != 6:
		return

	paths = map(lambda x: Path(x), filepaths)

	# --format R8G8B8A8_UNORM --assign-oetf srgb --levels 1 --cubemap PNG/right.png PNG/left.png PNG/top.png PNG/bottom.png PNG/front.png PNG/back.png output_cube.ktx && cp output_cube.ktx
	# /mnt/c/C/Personal/Disarray/App/Assets/Textures/cubemap_default.ktx

	args = ['ktx', 'create', '--format', 'R8G8B8A8_UNORM', '--assign-oetf', 'srgb', '--levels', '1', '--cubemap']
	for path in paths:
		args.append(str(Path(path).absolute()))

	args.append("output_cube.ktx")
	subprocess.call(args)


def main(base_name: str, file_or_image):
    output_base_name = base_name
    filepaths = extract_texture_map_faces(file_or_image, output_base_name)
    convert_to_ktx(filepaths)


def uri_validator(x):
    try:
        result: ParseResult = urlparse(x)
        return all([result.scheme, result.netloc])
    except:
        return False


def download_and_convert_image(url):
    # Send a GET request to the URL
    response = requests.get(url)

    # Check if the request was successful (status code 200)
    if response.status_code == 200:
        content_type = response.headers.get('Content-Type')

        if content_type != None and content_type.count('text/html') != 0:
            print(f"Invalid content type {content_type}.")
            return None

        # Open the image using Pillow from the response content
        image = Image.open(BytesIO(response.content))
        return image
    else:
        # If the request was not successful, print an error message
        print(f"Failed to download image. Status code: {response.status_code}")
        return None


if __name__ == "__main__":
	argument_parser = ArgumentParser()
	argument_parser.add_argument('-input_file', '-i', type=str, required=True)
	argument_parser.add_argument(
	    '-base_name', '-o', type=str, default='output_cubemap', required=False)
	namespace = argument_parser.parse_args()
	if 'input_file' not in namespace:
		exit(-1)

	input_file = namespace.input_file
	if uri_validator(namespace.input_file):
		parsed_uri = urlparse(namespace.input_file)
		input_file = download_and_convert_image(namespace.input_file)

	if input_file is None:
		exit(-1)

	if 'base_name' not in namespace:
		exit(-1)

	main(namespace.base_name, input_file)
