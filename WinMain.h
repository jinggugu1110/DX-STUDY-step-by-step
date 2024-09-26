#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#include<windows.h>
#include<d3d11.h>
#include<cassert>
#include<dxgi1_6.h>
#include<cmath>
#include<DirectXMath.h>
using namespace DirectX;

#include<fstream>

//�f�o�C�X�A�R���e�L�X�g-------------------------------------------------------
ID3D11Device* Device;//�o�b�t�@��o�b�t�@�r���[�A�p�C�v���C���X�e�[�g������֐�������
ID3D11DeviceContext* Context;//�o�b�t�@�r���[��p�C�v���C���X�e�[�g���p�C�v���C���ɃZ�b�g����֐�������
ID3D11Debug* Debug;
HRESULT Hr;
//���\�[�X-------------------------------------------------------------------
IDXGISwapChain1* SwapChain;
ID3D11RenderTargetView* BackBufferView;
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
ID3D11Buffer* PositionBuffer;
//�p�C�v���C��-------------------------------------------------------------------
ID3D11InputLayout* InputLayout;
ID3D11VertexShader* VertexShader;
ID3D11PixelShader* PixelShader;
ID3D11RasterizerState* RasterizerState;
D3D11_VIEWPORT Viewport;
//�E�B���h�E--------------------------------------------------------------------
LPCWSTR	WindowTitle = L"GAME TITLE";
int ClientWidth = 1280;
int ClientHeight = 720;
int DisplayWidth = GetSystemMetrics(SM_CXSCREEN);//������
int DisplayHeight = GetSystemMetrics(SM_CYSCREEN);//��������
int ClientLeft = (DisplayWidth - ClientWidth) / 2;//�����\��
int ClientTop = (DisplayHeight - ClientHeight) / 2;//�����\��
HBRUSH HBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//�w�i�F
#if 1 
DWORD WindowStyle = WS_OVERLAPPEDWINDOW;//�E�B���h�E�g����
#else
DWORD WindowStyle = WS_POPUP;//Alt + F4�ŕ���
#endif
HWND HWnd;//�E�B���h�E�n���h��

//�R���p�C���ς݃V�F�[�_��ǂݍ��ރt�@�C���o�b�t�@�N���X
class BIN_FILE11 {
public:
    BIN_FILE11(const char* fileName) :Succeeded(false) {
        std::ifstream ifs(fileName, std::ios::binary);
        if (ifs.fail()) {
            return;
        }
        Succeeded = true;
        std::istreambuf_iterator<char> first(ifs);
        std::istreambuf_iterator<char> last;
        Buffer.assign(first, last);
        ifs.close();
    }
    bool succeeded() const { return Succeeded; }
    const char* code() const { return Buffer.data(); }
    size_t size() const { return Buffer.size(); }
private:
    std::string Buffer;
    bool Succeeded;
};