import json

def json_parser_list_type(input_file, top):
    with open(input_file, encoding="utf-8") as f:
        y = json.load(f)
    
    m = y["modules"][top]
    type_list = []

    for cell, detail in m["cells"].items():
        type_list.append(detail["type"])
    
    print(list(dict.fromkeys(type_list)))


if __name__ == "__main__":
    print(json_parser_list_type("/mnt/nas/users/cam/LSOracle_mod/LSOracle/benchmarks/picorv32/picorv32_attr.json", "top"))