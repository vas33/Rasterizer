#pragma once
#include "d3d12.h"
#include "FrameResource.h"

template<typename T, bool isConstantBuffer>
class BufferCreator
{
public:
	BufferCreator(ID3D12Device* device, UINT elementCount)
		:m_IsConstantBuffer(isConstantBuffer)
	{
		m_ElementByteSize = sizeof(T);

		//todo calc offset for constant buffer

		ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_UploadBuffer)));

		ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
	}

	~BufferCreator()
	{
		if (m_UploadBuffer)
		{
			m_UploadBuffer->Unmap(0, nullptr);
		}
		m_MappedData = nullptr;
	}

	ID3D12Resource* Resource()const
	{
		return m_UploadBuffer.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&m_MappedData[elementIndex * m_ElementByteSize], &data, sizeof(T));
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
	BYTE* m_MappedData = nullptr;
	UINT m_ElementByteSize = 0;
	bool m_IsConstantBuffer = false;
};

inline UINT CalcBufferSizeWithRounding(UINT byteSize)
{
	// Constant buffers must be a multiple of the minimum hardware
// allocation size (usually 256 bytes).  So round up to nearest
// multiple of 256.  We do this by adding 255 and then masking off
// the lower 2 bytes which store all bits < 256.
// Example: Suppose byteSize = 300.
// (300 + 255) & ~255
// 555 & ~255
// 0x022B & ~0x00ff
// 0x022B & 0xff00
// 0x0200
// 512
	return (byteSize + 255) & ~255;
}



class Rasterizer
{
public:
	Rasterizer(ID3D12Device* device,
		UINT width, UINT height,
		DXGI_FORMAT format,
		UINT vbSize, UINT ibSize);

	void BuildDesrciptorsBase();
	void BuildResources();

	void AssignDescriptorsStart(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
		UINT descriptorSize);

	void Execute(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12PipelineState* computePSO,
		ID3D12Resource* inputVb,
		ID3D12Resource* inputIb,
		ID3D12Resource* constantBuffer = nullptr);

	void SetConstantBuffer(const SceneConstantBuffer& constBuffer);
	void OnResize(UINT newWidth, UINT newHeight);
private:
	void CreateConstantBuffers();
private:
	ID3D12Device* m_Device = nullptr;
	DXGI_FORMAT m_Format;
	UINT m_Width = 800;
	UINT m_Height = 600;
	UINT m_VBSize = 0;
	UINT m_IBSize = 0;

	//descriptors
	

	std::unique_ptr<BufferCreator<SceneConstantBuffer, true>> m_ConstantBuffer = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertBufferResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBufferResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_OutputResource = nullptr;


	CD3DX12_CPU_DESCRIPTOR_HANDLE m_ConstantsTableCPU;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_ConstantsTableGPU;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_VertexBufferCPUSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_VertexBufferGPUSrv;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_IndexBufferCPUSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_IndexBufferGPUSrv;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_OutputTextureCPUUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_OutputTextureGPUUav;
};