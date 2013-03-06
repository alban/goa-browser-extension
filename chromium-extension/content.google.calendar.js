console.log("goa: GOA content script for Google Calendar loading");
setupGoaIntegration (
  '//*[starts-with(@href, "https://profiles.google.com/")]//text()[contains(.,"@")]',
  ['calendar']
);
