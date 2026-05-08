# files

This is an basic encryption/decryption demo using xor. It shows proper encryption/decryption, as well as how output is changed when using the wrong key
## Run

Create a sample file to encrypt:

echo "This is my secret message for the security project." > secret.txt
cat secret.txt

Encrypt the file using a key:
python3 xor_encryptor.py encrypt secret.txt encrypted.bin mykey

View the encrypted file as raw bytes:
xxd encrypted.bin

Decrypt the file using the correct key:
python3 xor_encryptor.py decrypt encrypted.bin decrypted.txt mykey
cat decrypted.txt

Decrypt the file using the wrong key:
python3 xor_encryptor.py decrypt encrypted.bin wrong.txt wrongkey
xxd wrong.txt

## Screenshots

Location: /security/screenshots

# Contribution
  
Added xor_data function which encrypts/decrypts data using xor,

Added process_file function which handles the main logic of the demo. It reads the input file,
prepares the key, calls the xor_data function, then writes the output,

Added main function which handles the command arguments for the demo and ensure program runs smoothly
## References

1. [ChatGPT] Assited me with xor commands to use in demo.
2. [Xor Cipher](https://www.geeksforgeeks.org/dsa/xor-cipher/)