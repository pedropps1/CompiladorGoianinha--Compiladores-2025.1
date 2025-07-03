# Compilador e flags
CC = gcc
CFLAGS = -Wall -g

# Fontes do projeto
OBJS = goianinha.tab.o lex.yy.o ast.o tabela_simbolos.o semantico.o codigo.o main.o

# --- Adicionado para testes ---
# Diretório contendo os arquivos de teste
TEST_DIR = testes
# Lista todos os arquivos de teste .txt no diretório
TEST_FILES = $(wildcard $(TEST_DIR)/*.txt)

# Regra principal
goianinha: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Gera parser com Bison
goianinha.tab.c goianinha.tab.h: goianinha.y
	bison -d goianinha.y

# Gera scanner com Flex
lex.yy.c: goianinha.l goianinha.tab.h
	flex goianinha.l

# Compila objetos individuais
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Limpeza dos arquivos gerados
clean:
	rm -f goianinha *.o lex.yy.c goianinha.tab.c goianinha.tab.h saida.s

# Build completo do zero
all: clean goianinha


# Regra de teste CORRIGIDA
test: goianinha
	@echo "Iniciando a execução dos testes..."
	@if [ ! -d "$(TEST_DIR)" ]; then \
		echo "ERRO: Diretório '$(TEST_DIR)' não encontrado."; \
		exit 1; \
	fi
	@for test_file in $(TEST_FILES); do \
		echo "--- Executando teste: $$test_file ---"; \
		if ./goianinha $$test_file; then \
			echo "--- [SUCESSO] Compilação Goianinha OK. Executando SPIM... ---"; \
			spim -file saida.s; \
		else \
			echo "--- [FALHA] Erro na compilação Goianinha. SPIM não será executado. ---"; \
		fi; \
		echo "-----------------------------------------------------\n"; \
	done
	@echo "Todos os testes foram executados."


# Adiciona 'test' às regras que não geram arquivos
.PHONY: clean all test