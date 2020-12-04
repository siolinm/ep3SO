import sys


def geraArq(nome, tam):
    import random as rd
    import string as st

    f = open(nome, "a")
    for i in range(int(tam)):
        f.write(rd.choice(st.ascii_letters))
    f.close()


geraArq(sys.argv[1], sys.argv[2])
