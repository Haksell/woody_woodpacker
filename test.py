import os


TEMPLATE = """#include <stdio.h>

int main() {{
{}
}}
"""

for i in range(300, 400):
    # print(str(i).center(40, "="))
    filename = f"resources/hello{i}.c"
    open(filename, "w").write(
        TEMPLATE.format("\n".join(['    printf("Hello world\\n");'] * i))
    )
    os.system(f"gcc {filename} -no-pie -m64 -o resources/sample{i}")
    os.system(f"./woody_woodpacker resources/sample{i}; echo {i} $?")
