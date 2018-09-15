class MainView extends ViewDoc {
    prop headerContainer;
    prop detailContainer;
    
    prop headerView;
    
    fun initView() {
        if (this.headerContainer == null) {
            this.headerContainer = this.itemByID(ViewGroup, "header");
        }
        
        if (this.detailContainer == null) {
            this.detailContainer = this.itemByID(ViewGroup, "category_detail");
        }
        
        this.appendHeader();
    }
    
    fun appendHeader() {
        this.headerView = ViewDoc.createView(SearchHeader, "boyia://app/layout/search_header.html");
        this.headerView.initView();
        this.headerContainer.appendView(this.headerView);
    }
}