local = "C:/Users/OPTSOL Academy Kids/Documents/Pessoal/UFES/TCC2/iniciacao_cientifica/códigos/instancias/"

temporadas = ["21-22", "22-23", "23-24", "24-25"]
modalidades = ["masculina", "feminina"]

for modalidade in modalidades:
    for temporada in temporadas:

        arquivo_tabela = local + modalidade + "/tabela-oficial-" + temporada + "bruta.txt"
        print(arquivo_tabela)
        arquivo_times = local + modalidade + "/times-" + temporada + ".txt"
        print(arquivo_times)

        with open(arquivo_tabela, "r") as arquivo:
            _tabela = arquivo.read()

        tabela = _tabela.split()

        with open (arquivo_times, "r") as arquivo:
            _times = arquivo.read()
        times = _times.split()

        destino = local + modalidade + "/tabela-oficial-" + temporada + ".txt"
        with open(destino, "a") as destino:
            index = 0
            for time in tabela:
                index_time = times.index(time)
                cod = str(index_time + 1) + " "
                index += 1
                print(cod)
                destino.write(cod)
                if index % 2 == 0:
                    print()
                    cod = "\n"
                    destino.write(cod)

