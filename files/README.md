# files

This is a file carver program that creates a fake disk image, hides a png inside it, tnen recovers that png by scanning the raw bytes.
This simulates recovering a file from a disk image.

## Run

python3 file_carver.py

## Screenshots

Location: /files/screenshots

# Contribution

file_carver.cpp:    
    Added create_fake_disk_image() for disk image generation and embedding of png,
    Used open(DISK_IMAGE, "wb") to write raw binary data,
    Used os.urandom(100) to create random byte data before and after the embedded PNG,
    Added recover_png() to recover the png from the created image,
    Added PNG header and ending marker scanning,
    Printed details such as byte locations and the output filename.
## References

1. [ChatGPT] helped me through the code and found me the sources I used below
2. [Computer Forensics](https://toolcatalog.nist.gov/taxonomy/index.php?ff_id=9)
3. [The Python Code](https://thepythoncode.com/article/how-to-recover-deleted-file-with-python?)