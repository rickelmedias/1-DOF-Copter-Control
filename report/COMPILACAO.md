# Compilação do relatório

Arquivo principal: `main.tex`. Execute os comandos dentro da pasta `report/`.

## Tectonic (recomendado, baixa os pacotes sozinho)

```sh
tectonic main.tex --keep-logs --keep-intermediates
```

Na primeira execução precisa de rede. O estilo usa BibTeX (`abntex2-alf.bst`), que o Tectonic executa sozinho.

## XeLaTeX

Instale uma distribuição TeX com abnTeX2, memoir, BibTeX, TikZ, caption, tabularx, listings e float.

```sh
latexmk -xelatex main.tex
```

Sem `latexmk`:

```sh
xelatex main.tex
bibtex main
xelatex main.tex
xelatex main.tex
```

## Fonte

Com XeLaTeX/Tectonic o preâmbulo usa Arial se ela estiver instalada no sistema ou em `report/fonts/` (`arial.ttf`, `arialbd.ttf`, `ariali.ttf`, `arialbi.ttf`, não incluídos no repositório por serem fontes da Microsoft). Sem Arial, avisa e usa a fonte sans-serif disponível; o PDF de entrega deve ser gerado com Arial.

## Figuras

As figuras de resultados em `images/results/` e `images/hardware/` são geradas pelos scripts de `simulation/` (ver o `README.md` da raiz). Os PDFs gerados já estão versionados, então não é preciso rodar o Python para compilar.

## Fotos do protótipo final

Coloque `final_cima.jpg`, `final_lado.jpg` e `final_rodando.jpg` em `images/prototype/`. Sem elas, o relatório mostra um quadro reservado.
