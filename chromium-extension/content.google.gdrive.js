console.log("goa: GOA content script for GDrive loading");
setupGoaIntegration (
  '//*[starts-with(@href, "https://profiles.google.com/")]//text()[contains(.,"@")]',
  ['files']
);
