console.log("goa: GOA content script for GMail loading");
setupGoaIntegration (
  '//*[starts-with(@href,"https://plus.google.com/") or starts-with(@href,"https://profiles.google.com/")]/..//text()[contains(.,"@")]',
  ['mail', 'chat', 'contacts']
);
