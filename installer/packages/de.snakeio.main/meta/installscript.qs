   function Component() { // default constructor
   installer.installationFinished.connect(this, Component.prototype.installationFinished);
}

Component.prototype.createOperations = function() {
   component.createOperations();
   if(systemInfo.productType === "windows") {

      component.addOperation("CreateShortcut", "@TargetDir@/bin/SnakeIO.exe", "@StartMenuDir@/SnakeIO.lnk", "description=sneesnaaanake");

      component.addOperation("CreateShortcut", "@TargetDir@/bin/SnakeIO.exe", "@HomeDir@/Desktop/SnakeIO.lnk", "description=sneesnaaanake");

   }
}
Component.prototype.installationFinished = function()  {

}
