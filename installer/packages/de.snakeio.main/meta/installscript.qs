function Component()
{
	// default constructor
}

Component.prototype.createOperations = function()
{
	// call default implementation to actually install Fraktalgenerator.exe
	component.createOperations();
	
	if(systemInfo.productType === "windows") {
		component.addOperation("CreateShortcut", "@TargetDir@/bin/SnakeIO.exe",
							"@StartMenuDir@/SnakeIO.lnk",
							"description=SnakeIO");
		component.addOperation("CreateShortcut", "@TargetDir@/bin/SnakeIO.exe",
							"@HomeDir@/Desktop/SnakeIO.lnk",
							"description=SnakeIO");					
	
	}


}