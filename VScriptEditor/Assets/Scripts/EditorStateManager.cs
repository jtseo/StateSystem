namespace StateSystem
{
    public class EditorStateManager : VLStateManager
    {
        public override void StateFuncRegistor()
        {
            base.StateFuncRegistor();
            
            UxViewMain.StateFuncRegist();
            UxViewStateContent.StateFuncRegist();
            UxGroupID.StateFuncRegist();
            VScriptLink.StateFuncRegist();
            UxViewColumn.StateFuncRegist();
            UxViewColumnEditor.StateFuncRegist();
            //VScriptCosmosMenu.StateFuncRegist();
            VScriptLogHistory.StateFuncRegist();
        }
    }
}