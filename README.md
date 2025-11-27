Bem vindo a Hellshift

Video do jogo sendo executado: 



https://github.com/user-attachments/assets/922be55b-1ac1-4bd9-82a7-0d2ae7b11e85



A sequir estão instruções sobre implementação e uso so sistema para poder executar o jogo:

(Para Windows)======================================================

  1) Baixe o repositorio GitHub:
     - Baixe o codigo do git em forma de arquivo ZIP e em seguida o abra em algum lugar da sua maquina (Ex: Área de Trabalho)

  2) Tenha um compilador C com a biblioteca Raylib configurada:
     - Caso não tenha um compilador instalado, acesse: https://www.msys2.org/
     - Instale o MSYS2 na sua maquina
     - Procure a pasta MSYS2 que acabou de instalar e a abra
     - Abra o MSYS2 MinGW64
     - Isso vai abrir um terminal MinGW64, coloque o seguinte comando nele: pacman -S mingw-w64-x86_64-raylib
     - Isso instala a biblioteca Raylib
     - Esse passo só precisa ser feito uma vez
  
  3) Compile e rode o jogo:
       - No terminal MSYS2 MinGW64 acesse a pasta do jogo usando o comando cd (Ex: cd /c/Users/SEU_USUARIO/Área\ de\ Trabalho/hellshift/hellshift)
       - (Importante, dentro do repertorio github existe uma pasta também nomeada hellshift que contem os arquivos main.c, é essa a pasta que deve ser acessada)
       - Em seguida compile o jogo usando o comando: make
       - Por fim execute o jogo com o comando: ./hellshift.exe
       - Isso vai abrir uma janela aonde você terá acesso ao jogo
    
(Para Mac)======================================================
1) Ter estes pré-requisitos:

    - macOS com Xcode Command Line Tools instaladas

    - Homebrew instalado
    - raylib instalado via Homebrew
  
2)  Baixe o repositorio GitHub:
     - Baixe ou clone o repositório codigo do git em forma de arquivo ZIP e em seguida o abra em algum lugar da sua maquina (Ex: Área de Trabalho)
  
3) Compile e rode o jogo:
   - No terminal, conferir se está na pasta correta usando "cd hellshift"
   - Executar make clean
   - Executar make run
   - Abrirá uma janela com o jogo
   - Tentar zerar o jogo!


(Para Linux/WSL)======================================================
1) Ter estes pré-requisitos:
   1.1) Caso seja WSL:
     - Windows com WSL2 instalado (de preferência Windows 11 com suporte a GUI – WSLg)
      - Distribuição Ubuntu instalada no WSL
   Linux e wsl:
    - Instalar dependências: 
    sudo apt update
    sudo apt install build-essential git cmake pkg-config
    sudo apt install libraylib-dev

2)  Baixe o repositorio GitHub:
     - Baixe ou clone o repositório codigo do git em forma de arquivo ZIP e em seguida o abra em algum lugar da sua maquina (Ex: Área de Trabalho)


3) Compile e rode o jogo:
   - No terminal, conferir se está na pasta correta usando "cd hellshift"
   - Executar make clean
   - Executar make run
   - Abrirá uma janela com o jogo
   - Tentar zerar o jogo!
  
obs: Nosso Makefile identifica o sistema (Linux/Mac) e roda automaticamente a versão correta!
    
    
