   function Component() { // default constructor
   installer.installationFinished.connect(this, Component.prototype.installationFinished);
}

Component.prototype.createOperations = function() {
   component.createOperations();
   if(systemInfo.productType === "windows") {

      component.addOperation("CreateShortcut", "@TargetDir@/bin/SnakeIO.exe", "@StartMenuDir@/SnakeIO.lnk", "description=Snaaaaake....");

      component.addOperation("CreateShortcut", "@TargetDir@/bin/SnakeIO.exe", "@HomeDir@/Desktop/SnakeIO.lnk", "description=Snaaaaake....");

   }
}
Component.prototype.installationFinished = function()  {
   try {
       if (installer.isInstaller()) {
           if (installer.status == QInstaller.Success) {
               if (systemInfo.productType === "windows") {
                   // Execute SnakeIO after installation
                   if(installer.executeDetached("@TargetDir@/bin/SnakeIO.exe", ["show"]) !== 0) {
                       print("Failed to start the application!");
                   } else {
                       print("Application started successfully.");
                   }
               } else {
                   print("Not on Windows. Skipping application execution.");
               }
           } else {
               print("Installation was not successful. Application will not be started.");
           }
       } else {
           print("Not in installer mode. Application will not be started.");
       }
   } catch(e) {
       console.log(e);
   }

}
