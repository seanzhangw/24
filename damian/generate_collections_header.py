def convert_txt_to_struct_header(input_txt="collections_with_numofsols_difficultylevel.txt", output_header="collections_with_numofsols_difficultylevel.h"):
    with open(input_txt, "r") as f:
        lines = f.readlines()

    collections = []
    for line in lines:
        parts = line.strip().split()
        if len(parts) != 6:
            raise ValueError(f"Each line must have exactly 6 elements (5 ints + 1 label): {line}")
        nums = list(map(int, parts[:5]))
        label = parts[5]
        collections.append((nums, label))

    with open(output_header, "w") as f:
        f.write("// This file is auto-generated from collections_with_numofsols_difficultylevel.txt\n")
        f.write("#pragma once\n\n")
        f.write("typedef struct {\n")
        f.write("    int numbers[5];\n")
        f.write("    const char* label;\n")
        f.write("} Collection;\n\n")

        f.write("const Collection collections[] = {\n")
        for nums, label in collections:
            numbers_str = ", ".join(map(str, nums))
            f.write(f"    {{ {{ {numbers_str} }}, \"{label}\" }},\n")
        f.write("};\n\n")

        f.write(f"const int num_collections = {len(collections)};\n")

    print(f"Header file '{output_header}' generated with {len(collections)} collections.")

if __name__ == "__main__":
    convert_txt_to_struct_header()
