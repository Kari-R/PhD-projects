
# MAG Scorer

This program computes Gaussian BIC local scores from a given CSV data file.


## Instructions

1. Run `make` in the directory to compile the program.
2. You can run the program with no arguments, i.e., `./scorer`, to see useful information.

As an example, if the CSV file is `example.csv`, then one way to calculate scores for it would be: `./scorer example.csv --max-vars 5 --max-comp 4 --max-pars 6`

Note: The input CSV file must use spaces as delimiters and should not contain any extra information such as variable names.
