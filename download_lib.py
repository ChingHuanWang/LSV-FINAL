import gdown

abc_url = "https://drive.google.com/u/0/uc?id=12cF9nrxql8d1UNezmaI40f4F-aSws9BW&export=download"
gen_aag_url = "https://drive.google.com/u/0/uc?id=19fhg8Cjk8lHUP7ifXtACks1NVOPdddUd&export=download"

abc_output = "lib/libabc.a"
gen_aag_output = "lib/libgenaag.a"

gdown.download(abc_url, abc_output)
gdown.download(gen_aag_url, gen_aag_output)
