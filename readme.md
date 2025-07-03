
# Compilador para a Linguagem Goianinha

Projeto acadêmico da disciplina de **Compiladores** (UFG - Instituto de Informática, 2025/1), desenvolvido para implementar um compilador didático da linguagem **Goianinha**. Este repositório reúne as etapas do projeto, que evoluem desde a análise léxica, passando pela análise sintática e semântica, até a geração de código MIPS.

---

## Índice

- [Descrição do Projeto](#descrição-do-projeto)
- [Funcionalidades](#funcionalidades)
- [Como Compilar](#como-compilar)
- [Como Executar](#como-executar)
- [Como Rodar os Testes](#como-rodar-os-testes)
- [Dependências](#dependências)
- [Informações Técnicas](#informações-técnicas)
- [Referências](#referências)
- [Aviso sobre Plágio](#aviso-sobre-plágio)

---

## Descrição do Projeto

O objetivo deste projeto é construir, em etapas, um compilador funcional para a linguagem Goianinha. O projeto é dividido em três grandes partes:

1. **Análise Léxica e Tabela de Símbolos:**  
   Implementação do analisador léxico usando Flex e de uma pilha de tabelas de símbolos em C.

2. **Análise Sintática:**  
   Implementação do analisador sintático com Bison (ou YACC), integrando-o ao léxico.

3. **Análise Semântica & Geração de Código:**  
   Construção de árvore sintática abstrata (AST), análise semântica, verificação de tipos, escopo e geração de código em Assembly MIPS, executável no simulador SPIM.

---

## Funcionalidades

- **Análise Léxica:** Reconhecimento dos tokens da linguagem Goianinha, identificação de erros léxicos e gerenciamento de comentários.
- **Tabela de Símbolos:** Estrutura de dados eficiente para controle de escopos, funções, variáveis e parâmetros.
- **Análise Sintática:** Validação da gramática, detecção de erros sintáticos e reporte com o número da linha.
- **Análise Semântica:** Checagem de declarações, tipos, escopos e outras regras semânticas.
- **Geração de Código:** Conversão da árvore sintática abstrata para código MIPS.
- **Testes Automatizados:** Execução de diversos programas de teste com checagem de saída no simulador SPIM.


## Como Compilar

Basta rodar:

```bash
make all
````

Ou, para compilar a partir do zero (limpando arquivos gerados anteriormente):

```bash
make clean
make
```

O executável gerado será chamado `goianinha`.

---

## Como Executar

Para analisar um arquivo-fonte Goianinha:

```bash
./goianinha caminho/para/arquivo.txt
```

O analisador processará o arquivo, reportando erros sintáticos, léxicos ou semânticos, e gerará o arquivo de saída `saida.s` (Assembly MIPS).

---

## Como Rodar os Testes

Para executar todos os testes automáticos presentes na pasta `testes/`:

```bash
make test
```

O comando irá:

* Compilar o projeto (se necessário)
* Rodar cada arquivo `.txt` presente em `testes/` pelo compilador
* Se a compilação for bem-sucedida, executa o arquivo Assembly gerado pelo simulador SPIM

---

## Dependências

* **GCC** (compilador C)
* **Flex** (gerador de analisador léxico)
* **Bison** (gerador de analisador sintático)
* **SPIM** (simulador MIPS para testar o código Assembly)
* Ambiente Linux (os scripts e compilação são garantidos para Linux)

Instale via:

```bash
sudo apt-get install build-essential flex bison spim
```

---

## Informações Técnicas

* **Gramática completa da linguagem Goianinha**: disponível nos arquivos do projeto e detalhada nos relatórios anexos.
* **Tabela de símbolos**: suporte a escopos aninhados, pesquisa, inserção e remoção.
* **Análise semântica**: checagem de tipos, escopos e regras específicas da linguagem (ver PDFs para detalhes).
* **Mensagens de erro**: sempre iniciam com `ERRO:`, seguidas da descrição e linha do erro, conforme exigido nos enunciados.

---

## Referências

* \[Alfred Aho et al. Compiladores - Princípios, Técnicas e Ferramentas. Pearson Addison Wesley, 2ª edição, 2007.]
* \[Patterson, D. A., & Hennessy, J. L. Organização e Projeto de Computadores.]
* Materiais e slides fornecidos na plataforma da disciplina.

