#include"WinMain.h"
#include <WinUser.h>
#include <windowsx.h>
#include <cstdio> 

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ INT)
{
    //�E�B���h�E
    {
        //�E�B���h�E�N���X�o�^
        {
            WNDCLASSEX windowClass = {};
            windowClass.lpszClassName = L"GAME_WINDOW";
            windowClass.hbrBackground = HBackground;
            windowClass.hInstance = hInstance;
            windowClass.lpfnWndProc = WndProc;
            windowClass.hIcon = LoadIcon(NULL, IDC_ICON);
            windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
            windowClass.cbSize = sizeof(WNDCLASSEX);
            RegisterClassEx(&windowClass);
        }
        //�E�B���h�E�������ĕ\��
        {
            //�܂��A�\���ʒu�A�E�B���h�E�̑傫������
            RECT windowRect = { 0, 0, ClientWidth, ClientHeight };
            AdjustWindowRect(&windowRect, WindowStyle, FALSE);
            int windowLeft = ClientLeft + windowRect.left;
            int windowTop = ClientTop + windowRect.top;
            int windowWidth = windowRect.right - windowRect.left;
            int windowHeight = windowRect.bottom - windowRect.top;
            //���ɁA�E�B���h�E������
            HWnd = CreateWindowEx(
                NULL,//�g���X�^�C���Ȃ�
                L"GAME_WINDOW",
                WindowTitle,
                WindowStyle,
                windowLeft,
                windowTop,
                windowWidth,
                windowHeight,
                NULL,//�e�E�B���h�E�Ȃ�
                NULL,//���j���[�Ȃ�
                hInstance,
                NULL//�����E�B���h�E�Ȃ�
            );
            //�E�B���h�E�\��
            ShowWindow(HWnd, SW_SHOW);
        }
    }
    //�f�o�C�X�A�R���e�L�X�g
    {
        UINT creationFlags = 0;
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);
        Hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &Device,
            nullptr,
            &Context);
        assert(SUCCEEDED(Hr));

#ifdef _DEBUG
        //�f�o�b�O�C���^�[�t�F�C�X
        Hr = Device->QueryInterface(
            __uuidof(ID3D11Debug),
            reinterpret_cast<void**>(&Debug));
        assert(SUCCEEDED(Hr));
#endif
    }
    //���\�[�X
    {
        //�X���b�v�`�F�C��������(�o�b�N�o�b�t�@���܂�ł���)
        {
            //DXGI�t�@�N�g��������
            IDXGIFactory4* dxgiFactory;
            Hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
            assert(SUCCEEDED(Hr));

            //�X���b�v�`�F�C�����L�q���Ă���
            DXGI_SWAP_CHAIN_DESC1 desc = {};
            desc.Width = ClientWidth;
            desc.Height = ClientHeight;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            desc.BufferCount = 2;
            desc.SampleDesc.Count = 1;
            desc.Scaling = DXGI_SCALING_NONE;
            Hr = dxgiFactory->CreateSwapChainForHwnd(Device, HWnd, &desc, NULL, NULL,
                &SwapChain);
            assert(SUCCEEDED(Hr));

            //��������Ȃ�
            dxgiFactory->Release();
        }
        //�o�b�N�o�b�t�@�̃r���[������
        {
            //�X���b�v�`�F�C������o�b�N�o�b�t�@���擾
            ID3D11Texture2D* backBuffer;
            Hr = SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
            assert(SUCCEEDED(Hr));

            //�o�b�N�o�b�t�@�̃r���[������
            Hr = Device->CreateRenderTargetView(backBuffer, nullptr, &BackBufferView);
            assert(SUCCEEDED(Hr));

            //��������Ȃ�
            backBuffer->Release();
        }
        //���_�o�b�t�@-positions
        {
            //�R�p�`�̒��_�ʒu�f�[�^��p��
            XMFLOAT3 positions[] = {
                { -0.5f, -0.5f, 0.0f },//����
                { -0.5f,  0.5f, 0.0f },//����
                {  0.5f, -0.5f, 0.0f },//�E��
            };
            //�o�b�t�@�L�q
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(XMFLOAT3) * _countof(positions);//�z��̑S��
            desc.Usage = D3D11_USAGE_DEFAULT;                 //�f�t�H���g�A�N�Z�X
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;        //���_�o�b�t�@�Ƃ���GPU�ɕR�Â�
            desc.CPUAccessFlags = 0;                          //CPU�̃o�b�t�@�ւ̃A�N�Z�X����
            //�f�[�^
            D3D11_SUBRESOURCE_DATA data = {};
            data.pSysMem = positions;
            //�o�b�t�@���쐬
            Hr = Device->CreateBuffer(&desc, &data, &PositionBuffer);
            assert(SUCCEEDED(Hr));
        }
        //�p�C�v���C��
        {
            //�C���v�b�g���C�A�E�g �� �V�F�[�_�[
            {
                //���_�C���v�b�g���C�A�E�g���`���A�T�C�Y�����߂Ă���
                UINT slot0 = 0;
                D3D11_INPUT_ELEMENT_DESC Elements[]{
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                };
                UINT numElements = sizeof(Elements) / sizeof(Elements[0]);

                //�V�F�[�_�[�o�C�g�R�[�h�̓ǂݍ���
                BIN_FILE11 vs("assets\\VertexShader.cso");
                assert(vs.succeeded());
                BIN_FILE11 ps("assets\\PixelShader.cso");
                assert(ps.succeeded());

                //���_�C���v�b�g���C�A�E�g������
                Hr = Device->CreateInputLayout(Elements, numElements,
                    vs.code(), vs.size(), &InputLayout);
                assert(SUCCEEDED(Hr));

                //���_�V�F�[�_������
                Hr = Device->CreateVertexShader(vs.code(), vs.size(), NULL, &VertexShader);
                assert(SUCCEEDED(Hr));

                //�s�N�Z���V�F�[�_�[������
                Hr = Device->CreatePixelShader(ps.code(), ps.size(), NULL, &PixelShader);
                assert(SUCCEEDED(Hr));
            }
            //���X�^���C�U�[�X�e�[�g
            {
                D3D11_RASTERIZER_DESC desc;
                desc.FrontCounterClockwise = FALSE;
                desc.CullMode = D3D11_CULL_NONE;
                //desc.CullMode = D3D11_CULL_BACK;
                desc.DepthBias = 0;
                desc.DepthBiasClamp = 0;
                desc.SlopeScaledDepthBias = 0;
                desc.DepthClipEnable = FALSE;
                desc.ScissorEnable = FALSE;
                desc.MultisampleEnable = FALSE;
                desc.AntialiasedLineEnable = FALSE;
                desc.FillMode = D3D11_FILL_SOLID;
                desc.FillMode = D3D11_FILL_WIREFRAME;
                Device->CreateRasterizerState(&desc, &RasterizerState);
            }
            //�\���̈���쐬
            {
                Viewport.Width = static_cast<float>(ClientWidth);
                Viewport.Height = static_cast<float>(ClientHeight);
                Viewport.TopLeftX = 0;
                Viewport.TopLeftY = 0;
                Viewport.MinDepth = D3D11_MIN_DEPTH;// 0.0f
                Viewport.MaxDepth = D3D11_MAX_DEPTH;// 1.0f
            }
        }
    }
    
    //���C�����[�v
    while (true)
    {
        //�E�B���h�E���b�Z�[�W�̎擾�A���o
        {
         
            MSG msg = { 0 };
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) break;
                DispatchMessage(&msg);
                continue;
            }
        }
        //�o�b�N�o�b�t�@�̃N���A
        {
            //�o�b�N�o�b�t�@�������_�[�^�[�Q�b�g�ɐݒ�
            Context->OMSetRenderTargets(1, &BackBufferView, nullptr);
            //�����_�[�^�[�Q�b�g�r���[���w�肵���F�ŃN���A
            static float radian = 0.0f;
            float r = cos(radian) * 0.5f + 0.5f;
            radian += 0.001f;
            FLOAT clearColor[4] = { r, 0.25f, 0.5f, 1.0f };
            Context->ClearRenderTargetView(BackBufferView, clearColor);
        }
        //�o�b�N�o�b�t�@�֕`��
        {
            //���_�C���v�b�g���C�A�E�g���Z�b�g
            Context->IASetInputLayout(InputLayout);
            //�g�|���W�[���Z�b�g
            Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            //���_�o�b�t�@���Z�b�g
            ID3D11Buffer* buffers[] = { PositionBuffer };
            UINT stride[] = { sizeof(XMFLOAT3) };
            UINT offset[] = { 0 };
            Context->IASetVertexBuffers(0, 1, buffers, stride, offset);

            //���_�V�F�[�_���Z�b�g
            Context->VSSetShader(VertexShader, nullptr, 0);
            //���X�^���C�U�[���Z�b�g
            Context->RSSetState(RasterizerState);
            //�r���[�|�[�g���Z�b�g
            Context->RSSetViewports(1, &Viewport);
            //�s�N�Z���V�F�[�_���Z�b�g
            Context->PSSetShader(PixelShader, nullptr, 0);

            //�`��
            Context->Draw(3, 0);
        }
        //�o�b�N�o�b�t�@�̕\��
        {
            SwapChain->Present(1, 0);
        }
    }
    //���
    {
        BackBufferView->Release();
        SwapChain->Release();
        Context->Release();
        Device->Release();
        RasterizerState->Release();
        VertexShader->Release();
        PixelShader->Release();
        InputLayout->Release();

        PositionBuffer->Release();
#ifdef _DEBUG
        Debug->Release();
#endif
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_MOUSEMOVE: // Handle mouse movement
    {
        int xPos = GET_X_LPARAM(lp); // x POS 
        int yPos = GET_Y_LPARAM(lp); // y POS
        char title[100];
        sprintf_s(title, sizeof(title), "Mouse Position: (%d, %d)", xPos, yPos);
        SetWindowTextA(hwnd, title);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    case WM_CLOSE:			// �~�{�^���������ꂽ�ꍇ
    {
        int res1 = MessageBoxA(hwnd, "�Q�[�����I�����܂����H", "�m�F", MB_YESNO | MB_ICONQUESTION); // ���b�Z�[�W�{�b�N�X��\�����ă��[�U�[�Ɋm�F
        if (res1 == IDYES) {
            DestroyWindow(hwnd);
        }
        break;
    }
    }
   
}