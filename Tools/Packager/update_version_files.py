from Libs.Packager import *
from Libs.shared import *

def main():
	config = Get_Shared_Config()
	config.IncrementVersion	= True
	
	pack = Packager()
	pack.Run(config)

if __name__ == "__main__":
    main()