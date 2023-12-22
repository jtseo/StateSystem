using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class UxRenderBoard : MonoBehaviour
{
    public GameObject m_text_go;
    public GameObject m_image_go;
    // Start is called before the first frame update
    void Start()
    {
        Text txt = m_text_go.GetComponent<Text>();

        Texture2D tex = (Texture2D)txt.mainTexture;

        Image im = m_image_go.GetComponent<Image>();

        Sprite mySprite = Sprite.Create(tex, new Rect(0.0f, 0.0f, tex.width, tex.height), new Vector2(0.5f, 0.5f), 100.0f);

        im.overrideSprite = mySprite;
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
