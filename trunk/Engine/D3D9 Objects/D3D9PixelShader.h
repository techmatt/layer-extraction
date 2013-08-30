/*
D3D9PixelShader.h
Written by Matthew Fisher

D3D9PixelShader wraps the IDirect3DPixelShader9 object and an associated ID3DXConstantTable.
*/

#ifdef USE_D3D9

class D3D9PixelShader : public GraphicsAsset
{
public:
    D3D9PixelShader();
    ~D3D9PixelShader();
    
    void FreeMemory();
    void ReleaseMemory();
    void Reset(GraphicsDevice &graphics);

    void Init(GraphicsDevice &graphics, const String &filename);
    void Set() const;
    __forceinline void SetMatrix(const char *Name, const Matrix4 &M) const
    {
        D3DXMATRIX DM(M);
        D3DValidate( _constantTable->SetMatrix(_device, Name, &DM), "SetMatrix");
    }
    __forceinline void SetMatrix(const String &Str, const Matrix4 &M) const
    {
        SetMatrix(Str.CString(), M);
    }
    __forceinline void SetFloat(const char *Name, float Value) const
    {
        D3DValidate( _constantTable->SetFloat(_device, Name, Value), "SetFloat" );
    }
    __forceinline void SetFloat(const String &Str, float Value) const
    {
        SetFloat(Str.CString(), Value);
    }
    __forceinline void SetVec3(const char *Name, const Vec3f &V) const
    {
        float V3[3] = {V.x, V.y, V.z};
        D3DValidate( _constantTable->SetFloatArray(_device, Name, V3, 3), "SetFloatArray" );
    }
    __forceinline void SetVec3(const String &Str, const Vec3f &V) const
    {
        SetVec3(Str.CString(), V);
    }
    __forceinline void SetVec4(const char *Name, const Vec4f &V) const
    {
        float V4[4] = {V.x, V.y, V.z, V.w};
        D3DValidate( _constantTable->SetFloatArray(_device, Name, V4, 4), "SetFloatArray" );
    }
    __forceinline void SetVec4(const String &Str, const Vec4f &V) const
    {
        SetVec4(Str.CString(), V);
    }
    __forceinline void SetBool(const char *Name, bool Value) const
    {
        BOOL NewValue = FALSE;
        if(Value)
        {
            NewValue = TRUE;
        }
        D3DValidate( _constantTable->SetBool(_device, Name, NewValue), "SetBool" );
    }
    __forceinline void SetBool(const String &Str, bool Value) const
    {
        SetBool(Str.CString(), Value);
    }

private:
    String                  _shaderFile;
    LPDIRECT3DPIXELSHADER9  _shader;
    LPDIRECT3DDEVICE9       _device;
    LPD3DXCONSTANTTABLE     _constantTable;
};

#endif