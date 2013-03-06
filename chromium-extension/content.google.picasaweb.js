console.log("goa: GOA content script for Picasa loading");
setupGoaIntegration (
  '//*[starts-with(@href, "https://profiles.google.com/")]//text()[contains(.,"@")]',
  ['photos']
);
