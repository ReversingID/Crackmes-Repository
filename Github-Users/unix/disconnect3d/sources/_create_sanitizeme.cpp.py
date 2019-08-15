import random
import os

keys = []
keys_length = 100

first = "sometimes it works"
first_keys_used = 25

second = "sometimes it doesn't"
second_keys_used = 50

last = 'password is: {}'.format(os.environ['PASS'])
last_keys_used = 100


for i in range(keys_length):
    random_values_for_key = []
    for j in range(len(last)):
        random_values_for_key.append(random.randint(20, 100))
    keys.append(random_values_for_key)

# The printed code has to be copied into a c++ file

print("#include <iostream>")
print()
print("using namespace std;")
print()
print("const int keys[][22] = \\")
print(str(keys).replace("[", "{").replace("]", "}") + ";")

reversed_keys = list(reversed(keys))
def encoded_array(s, keys_used):
    coded_pwd = [ord(i) for i in s]
    for key in reversed_keys[:keys_used]:  # using reversed, so c++ keys will be used for "decoding"
        coded_pwd = [char ^ k for char, k in zip(coded_pwd, key)]

    return "{" + ', '.join(str(i) for i in coded_pwd) + "}"


print("int first[] = {};".format(encoded_array(first, first_keys_used)))
print("int second[] = {};".format(encoded_array(second, second_keys_used)))
print("int last[] = {};".format(encoded_array(last, last_keys_used)))


print(
"""
std::string decode(int* encoded_string, int keys_offset, int keys_used, int str_len)
{
    for(int i=keys_offset; i<keys_offset+keys_used; ++i)
        for(int j=0; j<str_len; ++j)
            encoded_string[j] ^= keys[i][j];

    std::string s;
    for(int j=0; j<str_len; ++j)
        s += static_cast<char>(encoded_string[j]);

    return s;
}

int main()
{
""")

print('    cout << "{}" << endl;'.format(os.environ.get('WELCOME', 'cr4ckm3. enjoy')))
print('    cout << "In this level, there\'s something wrong with decoding messages - you have to find the issue and fix code." << endl;')
print('    cout << "You can find the issue with g++/clang++ sanitizers. For more info check man or documentation." << endl;')
print('    cout << "Example usage of sanitizers: g++ -fsanitize=address <source_file>" << endl;')
print('    cout << "Decoding messages:" << endl;')
print("    cout << decode(first, {}, {}, {}) << endl;".format(keys_length - first_keys_used, first_keys_used, len(first))); 
print("    cout << decode(second, {}, {}, {}) << endl;".format(keys_length - second_keys_used, second_keys_used, len(second))); 
print("    cout << decode(last, {}, {}, {}) << endl;".format(keys_length - last_keys_used, last_keys_used+1, len(last))); 

print("}")
