#!/usr/bin/env python3
"""
ESP32-S3 LittleFS Filesystem Image Creator
Creates a LittleFS image from a local directory and uploads it to ESP32-S3
"""

import os
import sys
import argparse
import subprocess
from pathlib import Path

try:
    from littlefs import LittleFS  # Requires: pip install littlefs-python
except ImportError:
    print("Error: littlefs-python package not installed.")
    print("Install it with: pip install littlefs-python")
    sys.exit(1)


def create_littlefs_image(source_dir, output_file, block_size=4096, block_count=128):
    """
    Create a LittleFS filesystem image from a source directory

    Args:
        source_dir: Path to source directory containing files to include
        output_file: Path where the LittleFS image will be written
        block_size: Filesystem block size (default 4096 for ESP32-S3)
        block_count: Number of blocks (default 128 = 512KB for your partition)
    """

    if not os.path.exists(source_dir):
        print(f"Error: Source directory '{source_dir}' does not exist")
        return False

    try:
        fs = LittleFS(block_size=block_size, block_count=block_count)

        source_path = Path(source_dir)
        file_count = 0

        for file_path in source_path.rglob('*'):
            if file_path.is_file():
                relative_path = file_path.relative_to(source_path)
                fs_path = str(relative_path).replace(
                    '\\', '/')

                print(f"Adding: {fs_path}")

                with open(file_path, 'rb') as f:
                    file_data = f.read()

                dir_path = os.path.dirname(fs_path)
                if dir_path and not dir_path == '.':
                    try:
                        fs.mkdir(dir_path)
                    except Exception:
                        pass

                with fs.open(fs_path, 'wb') as fh:
                    fh.write(file_data)

                file_count += 1

        with open(output_file, 'wb') as fh:
            fh.write(fs.context.buffer)

        print(f"Successfully created LittleFS image: {output_file}")
        print(f"Added {file_count} files from '{source_dir}'")
        print(f"Image size: {os.path.getsize(output_file)} bytes")

        return True

    except Exception as e:
        print(f"Error creating LittleFS image: {e}")
        import traceback
        traceback.print_exc()
        return False


def upload_to_esp32(image_file, port=None, offset=0x110000):
    """
    Upload LittleFS image to ESP32-S3 using esptool.py

    Args:
        image_file: Path to LittleFS image file
        port: Serial port (if None, esptool will try to auto-detect)
        offset: Flash offset address for LittleFS partition
    """

    if not os.path.exists(image_file):
        print(f"Error: Image file '{image_file}' does not exist")
        return False

    # Based on your partition table:
    # nvs:      0x9000  + 0x6000  = 0xF000
    # phy_init: 0xF000  + 0x1000  = 0x10000
    # factory:  0x10000 + 0x100000 = 0x110000
    # lfs:      starts at 0x110000 (after factory app)

    cmd = [
        'esptool.py',
        '--chip', 'esp32s3',
        '--baud', '921600',
        'write_flash',
        '--flash_mode', 'dio',
        '--flash_size', '4MB',
        '--flash_freq', '80m',
        str(offset),
        image_file
    ]

    if port:
        cmd.insert(3, port)
        cmd.insert(3, '--port')

    try:
        print(
            f"Uploading LittleFS image to ESP32-S3 at offset 0x{offset:X}...")
        print(f"Command: {' '.join(cmd)}")
        result = subprocess.run(
            cmd, check=True, capture_output=True, text=True)
        print("Upload successful!")
        print(result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Upload failed with error: {e}")
        print(f"Command: {' '.join(cmd)}")
        if e.stderr:
            print(f"Error output: {e.stderr}")
        return False
    except FileNotFoundError:
        print("Error: esptool.py not found. Make sure it's installed and in your PATH")
        print("Install with: pip install esptool")
        return False


def main():
    parser = argparse.ArgumentParser(
        description='Create and upload LittleFS image to ESP32-S3')
    parser.add_argument('--source', '-s', default='assets',
                        help='Source directory (default: assets)')
    parser.add_argument('--output', '-o', default='littlefs.bin',
                        help='Output image filename (default: littlefs.bin)')
    parser.add_argument('--block-size', type=int, default=4096,
                        help='Filesystem block size (default: 4096)')
    parser.add_argument('--block-count', type=int, default=128,
                        help='Number of blocks (default: 128 = 512KB)')
    parser.add_argument('--port', '-p',
                        help='Serial port (default: auto-detect)')
    parser.add_argument('--offset', type=lambda x: int(x, 0), default=0x110000,
                        help='Flash offset (default: 0x110000)')
    parser.add_argument('--create-only', action='store_true',
                        help='Only create image, do not upload')
    parser.add_argument('--upload-only', action='store_true',
                        help='Only upload existing image, do not create')

    args = parser.parse_args()

    os.remove(args.output) if os.path.exists(args.output) else None

    if not args.upload_only:
        print(f"Creating LittleFS image from '{args.source}'...")
        success = create_littlefs_image(
            args.source,
            args.output,
            args.block_size,
            args.block_count
        )
        if not success:
            sys.exit(1)

    if not args.create_only:
        success = upload_to_esp32(args.output, args.port, args.offset)
        if not success:
            sys.exit(1)


if __name__ == '__main__':
    main()
