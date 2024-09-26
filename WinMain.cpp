#include"WinMain.h"
#include <WinUser.h>
#include <windowsx.h>
#include <cstdio> 

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ INT)
{
    //ウィンドウ
    {
        //ウィンドウクラス登録
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
        //ウィンドウをつくって表示
        {
            //まず、表示位置、ウィンドウの大きさ調整
            RECT windowRect = { 0, 0, ClientWidth, ClientHeight };
            AdjustWindowRect(&windowRect, WindowStyle, FALSE);
            int windowLeft = ClientLeft + windowRect.left;
            int windowTop = ClientTop + windowRect.top;
            int windowWidth = windowRect.right - windowRect.left;
            int windowHeight = windowRect.bottom - windowRect.top;
            //つぎに、ウィンドウをつくる
            HWnd = CreateWindowEx(
                NULL,//拡張スタイルなし
                L"GAME_WINDOW",
                WindowTitle,
                WindowStyle,
                windowLeft,
                windowTop,
                windowWidth,
                windowHeight,
                NULL,//親ウィンドウなし
                NULL,//メニューなし
                hInstance,
                NULL//複数ウィンドウなし
            );
            //ウィンドウ表示
            ShowWindow(HWnd, SW_SHOW);
        }
    }
    //デバイス、コンテキスト
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
        //デバッグインターフェイス
        Hr = Device->QueryInterface(
            __uuidof(ID3D11Debug),
            reinterpret_cast<void**>(&Debug));
        assert(SUCCEEDED(Hr));
#endif
    }
    //リソース
    {
        //スワップチェインをつくる(バックバッファを含んでいる)
        {
            //DXGIファクトリをつくる
            IDXGIFactory4* dxgiFactory;
            Hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
            assert(SUCCEEDED(Hr));

            //スワップチェインを記述してつくる
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

            //もういらない
            dxgiFactory->Release();
        }
        //バックバッファのビューをつくる
        {
            //スワップチェインからバックバッファを取得
            ID3D11Texture2D* backBuffer;
            Hr = SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
            assert(SUCCEEDED(Hr));

            //バックバッファのビューをつくる
            Hr = Device->CreateRenderTargetView(backBuffer, nullptr, &BackBufferView);
            assert(SUCCEEDED(Hr));

            //もういらない
            backBuffer->Release();
        }
        //頂点バッファ-positions
        {
            //３角形の頂点位置データを用意
            XMFLOAT3 positions[] = {
                { -0.5f, -0.5f, 0.0f },//左下
                { -0.5f,  0.5f, 0.0f },//左上
                {  0.5f, -0.5f, 0.0f },//右下
            };
            //バッファ記述
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(XMFLOAT3) * _countof(positions);//配列の全長
            desc.Usage = D3D11_USAGE_DEFAULT;                 //デフォルトアクセス
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;        //頂点バッファとしてGPUに紐づけ
            desc.CPUAccessFlags = 0;                          //CPUのバッファへのアクセス拒否
            //データ
            D3D11_SUBRESOURCE_DATA data = {};
            data.pSysMem = positions;
            //バッファを作成
            Hr = Device->CreateBuffer(&desc, &data, &PositionBuffer);
            assert(SUCCEEDED(Hr));
        }
        //パイプライン
        {
            //インプットレイアウト と シェーダー
            {
                //頂点インプットレイアウトを定義し、サイズを求めておく
                UINT slot0 = 0;
                D3D11_INPUT_ELEMENT_DESC Elements[]{
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                };
                UINT numElements = sizeof(Elements) / sizeof(Elements[0]);

                //シェーダーバイトコードの読み込み
                BIN_FILE11 vs("assets\\VertexShader.cso");
                assert(vs.succeeded());
                BIN_FILE11 ps("assets\\PixelShader.cso");
                assert(ps.succeeded());

                //頂点インプットレイアウトをつくる
                Hr = Device->CreateInputLayout(Elements, numElements,
                    vs.code(), vs.size(), &InputLayout);
                assert(SUCCEEDED(Hr));

                //頂点シェーダをつくる
                Hr = Device->CreateVertexShader(vs.code(), vs.size(), NULL, &VertexShader);
                assert(SUCCEEDED(Hr));

                //ピクセルシェーダーをつくる
                Hr = Device->CreatePixelShader(ps.code(), ps.size(), NULL, &PixelShader);
                assert(SUCCEEDED(Hr));
            }
            //ラスタライザーステート
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
            //表示領域を作成
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
    
    //メインループ
    while (true)
    {
        //ウィンドウメッセージの取得、送出
        {
         
            MSG msg = { 0 };
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) break;
                DispatchMessage(&msg);
                continue;
            }
        }
        //バックバッファのクリア
        {
            //バックバッファをレンダーターゲットに設定
            Context->OMSetRenderTargets(1, &BackBufferView, nullptr);
            //レンダーターゲットビューを指定した色でクリア
            static float radian = 0.0f;
            float r = cos(radian) * 0.5f + 0.5f;
            radian += 0.001f;
            FLOAT clearColor[4] = { r, 0.25f, 0.5f, 1.0f };
            Context->ClearRenderTargetView(BackBufferView, clearColor);
        }
        //バックバッファへ描画
        {
            //頂点インプットレイアウトをセット
            Context->IASetInputLayout(InputLayout);
            //トポロジーをセット
            Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            //頂点バッファをセット
            ID3D11Buffer* buffers[] = { PositionBuffer };
            UINT stride[] = { sizeof(XMFLOAT3) };
            UINT offset[] = { 0 };
            Context->IASetVertexBuffers(0, 1, buffers, stride, offset);

            //頂点シェーダをセット
            Context->VSSetShader(VertexShader, nullptr, 0);
            //ラスタライザーをセット
            Context->RSSetState(RasterizerState);
            //ビューポートをセット
            Context->RSSetViewports(1, &Viewport);
            //ピクセルシェーダをセット
            Context->PSSetShader(PixelShader, nullptr, 0);

            //描画
            Context->Draw(3, 0);
        }
        //バックバッファの表示
        {
            SwapChain->Present(1, 0);
        }
    }
    //解放
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
    case WM_CLOSE:			// ×ボタンが押された場合
    {
        int res1 = MessageBoxA(hwnd, "ゲームを終了しますか？", "確認", MB_YESNO | MB_ICONQUESTION); // メッセージボックスを表示してユーザーに確認
        if (res1 == IDYES) {
            DestroyWindow(hwnd);
        }
        break;
    }
    }
   
}