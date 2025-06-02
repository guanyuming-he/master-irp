./plan/plan.pdf : ./plan/plan.tex ./plan/references.bib ./plan/report-template.cls
	latexmk -pdf -cd ./plan/plan.tex
./thesis/thesis.pdf : ./thesis/thesis.tex ./thesis/references.bib ./thesis/report-template.cls
	latexmk -pdf -cd ./thesis/thesis.tex

./gh124-project-plan.pdf : ./plan/plan.pdf
	cp ./plan/plan.pdf ./gh124-project-plan.pdf
./gh124-final-report.pdf : ./thesis/thesis.pdf
	cp ./thesis/thesis.pdf ./gh124-final-report.pdf

.PHONY : all
all : ./gh124-project-plan.pdf ./gh124-final-report.pdf
