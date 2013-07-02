console.log("goa: GOA content script for Google Plus loading");
setupGoaIntegration (
  '//li[starts-with(@guidedhelpid,"g")]/div[starts-with(@guidedhelpid,"g")]/div/div/div/span/text()[contains(.,"@")]',
  ['chat', 'contacts']
);
