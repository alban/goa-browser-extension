console.log("goa: GOA content script for GMail loading");
setupGoaIntegration (
  '//*[@role="navigation"]//*[starts-with(@href, "https://profiles.google.com/")]//text()[contains(.,"@")]',
  ['mail']
);
