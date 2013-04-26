console.log("goa: GOA content script for GMail loading");
setupGoaIntegration (
  '//*[@role="navigation"]//text()[contains(.,"@")]',
  ['mail', 'chat', 'contacts']
);
