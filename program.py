#!/usr/bin/python3

import time
import sys
import os

if len(sys.argv) == 1:
    print("Usage: " + sys.argv[0] + " <Options>")
    sys.exit(1)

if sys.argv[1] == "help":
    print("Options:")
    print("")
    print("\twait\t\tWaits for a sec")
    print("\twaitquarter\tWaits for a quarter sec")
    print("\tgreet\t\tSays hello")
    print("\tgreetlong\tSays a long hello")
    print("\ttab\t\tPrints a tab character")
    print("\ttext\t\tPrints a text")
    print("\tgenerate_file\tCreates a file")
    print("\tremove_file\tRemoves a file")
elif sys.argv[1] == "wait":
    time.sleep(1)
elif sys.argv[1] == "waitquarter":
    time.sleep(0.25)
elif sys.argv[1] == "greet":
    print("Helllo!")
elif sys.argv[1] == "greetlong":
    print("Helllioaeeraeosincoooooooooooooooooooooooooooooooooooooo!")
elif sys.argv[1] == "greetname":
    if len(sys.argv) == 3:
        print(f"Helllo {sys.argv[2]}!")
elif sys.argv[1] == "tab":
    print("\t")
elif sys.argv[1] == "env":
    if "HELLO" in os.environ:
        print("1")
    else:
        print("0")
elif sys.argv[1] == "envlist":
    print("\n".join(os.environ))
elif sys.argv[1] == "envlist2":
    if len(sys.argv) == 3:
        print(sys.argv[2] in "\n".join(os.environ))
elif sys.argv[1] == "generate_file":
    with open("/tmp/program.txt", "wt") as f:
        f.write("A really important revolutionary program.")
elif sys.argv[1] == "remove_file":
    if os.path.exists("/tmp/program.txt"):
        os.remove("/tmp/program.txt")
elif sys.argv[1] == "text":
    print("Line 1\nLine 2\nLine 3\nLine 4\nprogram.py the program\n")
