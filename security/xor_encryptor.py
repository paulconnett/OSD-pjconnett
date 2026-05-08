import sys


def xor_data(data, key):
    """
    Encrypts or decrypts data using XOR.
    """
    result = bytearray()

    for i in range(len(data)):
        result.append(data[i] ^ key[i % len(key)])

    return bytes(result)


def process_file(mode, input_file, output_file, key_text):
    """
    Encryption/Decryption handleing. Read/call xor_data, write to output
    """
    key = key_text.encode()

    if len(key) == 0:
        print("Error: key cannot be empty.")
        sys.exit(1)

    with open(input_file, "rb") as file:
        data = file.read()

    processed_data = xor_data(data, key)

    with open(output_file, "wb") as file:
        file.write(processed_data)

    if mode == "encrypt":
        print("File encrypted successfully.")
        print("Input file:", input_file)
        print("Encrypted file:", output_file)
    else:
        print("File decrypted successfully.")
        print("Input file:", input_file)
        print("Decrypted file:", output_file)


def main():
    """
    Handles command line arguments.
    """
    if len(sys.argv) != 5:
        print("Usage:")
        print("  python3 xor_encryptor.py encrypt input.txt output.bin key")
        print("  python3 xor_encryptor.py decrypt input.bin output.txt key")
        sys.exit(1)

    mode = sys.argv[1]
    input_file = sys.argv[2]
    output_file = sys.argv[3]
    key_text = sys.argv[4]

    if mode != "encrypt" and mode != "decrypt":
        print("Error: mode must be encrypt or decrypt.")
        sys.exit(1)

    process_file(mode, input_file, output_file, key_text)


if __name__ == "__main__":
    main()