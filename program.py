#!/usr/bin/python3

import time
import sys

if len(sys.argv) == 1:
    print("Usage: " + sys.argv[0] + " <Options>")
    sys.exit(1)

if sys.argv[1] == "help":
    print("Options:")
    print("")
    print("\twait\t\tWaits for a sec")
    print("\twaitquarter\tWaits for a quarter sec")
    print("\tgreet\t\tSays hello")
    print("\ttab\t\tPrints a tab character")
elif sys.argv[1] == "wait":
    time.sleep(1)
elif sys.argv[1] == "waitquarter":
    time.sleep(0.25)
elif sys.argv[1] == "greet":
    print("Helllo!")
elif sys.argv[1] == "greetname":
    if len(sys.argv) != 3:
        print(f"Helllo {sys.argv[2]}!")
elif sys.argv[1] == "tab":
    print("\t")
