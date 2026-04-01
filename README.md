# Simulação de Cidade do Velho Oeste | Projeto Final

## Sumário

 - [Descrição](#descrição)
    - O que o código faz
    - Imagens do programa
 - [Instalação e Execução](#instalação-e-execução)
    - Instruções de requisitos, como instalar, compilar e executar
 - [Problemas Encontrados](#problemas-encontrados)
 - [Melhorias Futuras](#melhorias-futuras)
 - [Elementos das Atividades Práticas](#elementos-das-atividades-práticas)
 - [Trabalhos dos Integrantes](#trabalhos-dos-integrantes)

## Descrição

## Instalação e Execução

1. Após baixar o arquivo compactado ``icg_project-main.zip``, extraia seu conteúdo.

2. Depois de descompactar, você terá um diretório nomeado ``icg_project-main``.

3. Acesse as pastas relevantes por meio de seu terminal ``$ cd icg_project-main\src``.

4. Certifique-se que as dependências (diretórios ``predios``, ``texturas`` e arquivos ``stb_image.h``, ``tinyobj_loader_c.h`` e ``script_fbx_to_obj_blender.py``) estejam em src.

5. Após isso, execute em seu terminal o comando ``$ g++ projeto_icg.c -o projeto_icg -lGL -lGLU -lglut && ./projeto_icg``.

6. Após executar, a janela com o ambiente deve aparecer. É possível se mover no mundo usando as teclas: W, A, S, D e controlar a câmera com Q, E.

## Problemas Encontrados

## Melhorias Futuras

## Elementos das Atividades Práticas

Atividade 2: Uso e manipulação do gluLookAt, gluPerspective e glFrustum e uso de glPushMatrix e glPopMatrix na criação de objetos e modelos no mundo.

Atividade 3: Utilização do Z-Buffer para lidar com o problema de oclusão.

Atividade 4: Adição de fonte de luz, no caso implementado, representando o sol da cena.

Atividade 5: Adição de texturas para o chão, caminho principal, bola de feno e céu.

Atividade 6: Uso de spline de bèzier para implementar o movimento utilizado na bola de feno.

## Trabalhos dos Integrantes

[@ArthurH35](https://github.com/ArthurH35):
- Construção do layout da cidade.
- Adição de elementos e modelos para ambientação.
- Implementação do movimento da câmera.
- Implementação das splines usadas para adicionar movimento a um dos objetos.

[@LudmilaGomes](https://github.com/LudmilaGomes):
- Adaptação dos modelos 3D das estruturas a partir de fonte open source.
- Configuração para importar os modelos no código do projeto.
- Adição de texturas e iluminação.
- Construção do interior de uma das estruturas.
