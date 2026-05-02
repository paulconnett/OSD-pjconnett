import os

PNG_HEADER = b"\x89PNG\r\n\x1a\n"
PNG_END = b"IEND\xaeB`\x82"

DISK_IMAGE = "fake_disk.img"
RECOVERED_FILE = "recovered.png"

def create_fake_disk_image():
    hidden_png = (
        b"\x89PNG\r\n\x1a\n"
        b"\x00\x00\x00\rIHDR"
        b"\x00\x00\x00\x01"
        b"\x00\x00\x00\x01"
        b"\x08\x06\x00\x00\x00"
        b"\x1f\x15\xc4\x89"
        b"\x00\x00\x00\rIDAT"
        b"\x78\x9cc\xf8\xff\xff?\x00\x05\xfe\x02\xfe"
        b"\xdc\xccY\xe7"
        b"\x00\x00\x00\x00IEND\xaeB`\x82"
    )

    with open(DISK_IMAGE, "wb") as file:
        file.write(os.urandom(100))
        file.write(hidden_png)
        file.write(os.urandom(100))

    print("Fake disk image created:", DISK_IMAGE)


def recover_png():
    with open(DISK_IMAGE, "rb") as file:
        disk_data = file.read()

    start = disk_data.find(PNG_HEADER)

    if start == -1:
        print("No PNG header found.")
        return

    end = disk_data.find(PNG_END, start)

    if end == -1:
        print("No PNG ending marker found.")
        return

    end = end + len(PNG_END)

    recovered_data = disk_data[start:end]

    with open(RECOVERED_FILE, "wb") as file:
        file.write(recovered_data)

    print("PNG header found at byte:", start)
    print("PNG ending marker found at byte:", end)
    print("Recovered PNG saved as:", RECOVERED_FILE)


def main():
    create_fake_disk_image()
    recover_png()


if __name__ == "__main__":
    main()