using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.IO;
using Newtonsoft.Json;
using GameMonitor.Config;

namespace GameMonitor.Connection
{
    public enum GameConnectionState
    {
        Disconnected,
        Connecting,
        Connected
    }

    public class PacketWrapper
    {
        public string ClassName = "";
        public string InnerJSON = "";
    }

    public class QueuedPacketResponseEvent
    {
        public int RequestID = 0;
        public PacketResponseEvent Event = null; 
    }

    public delegate void PacketResponseEvent(GameConnection Connection, BasePacket Packet);

    public class GameConnection
    {
        public GameConnectionState State = GameConnectionState.Disconnected;
        public ConnectionConfig Configuration = null;
        public const float ConnectionFailureBackoff = 5000.0f;
        public GameState CurrentGameState = new GameState();
        public bool StateChangeDirty = false;

        private int     m_lastTickCount = 0;
        private float   m_stateTime = 0.0f;
        private float   m_connectionBackOffTimer = 0.0f;

        private Socket                  m_socket = null;
        private SocketAsyncEventArgs    m_connect_args = null;

        private List<QueuedPacketResponseEvent> m_queuedResponseEvents = new List<QueuedPacketResponseEvent>();

        private bool m_recieving_payload = false;
        private uint m_payload_size = 0;
        private byte[] m_payload_buffer = null;
        private int m_payload_buffer_read = 0;

        public GameConnection(ConnectionConfig config)
        {
            Configuration = config;
            StateChangeDirty = true;
        }

        ~GameConnection()
        {
            m_socket.Close();
            m_socket = null;
        }

        public bool HasStateChanged()
        {
            bool result = StateChangeDirty;
            StateChangeDirty = false;
            return result;
        }

        private void ChangeState(GameConnectionState NewState)
        {
            State = NewState;
            StateChangeDirty = true;
            m_stateTime = 0.0f;
        }

        public void Poll()
        {
            int elapsed = Environment.TickCount - m_lastTickCount;
            m_lastTickCount = Environment.TickCount;
            GameConnectionState originalState = State;

            switch (State)
            {
                case GameConnectionState.Disconnected:
                    {
                        m_connectionBackOffTimer -= elapsed;
                        if (m_connectionBackOffTimer <= 0.0f)
                        {
                            ChangeState(GameConnectionState.Connecting);
                        }
                        break;
                    }
                case GameConnectionState.Connecting:
                    {
                        if (m_stateTime == 0.0f)
                        {
                            m_socket = new Socket(SocketType.Stream, ProtocolType.Tcp);
                            m_socket.NoDelay = true;
                            m_socket.LingerState.Enabled = false;

                            m_connect_args = new SocketAsyncEventArgs();
                            m_connect_args.Completed += AsyncConnectCompleted;
                            m_connect_args.RemoteEndPoint = new IPEndPoint(IPAddress.Parse(Configuration.IP), Configuration.Port);

                            System.Console.WriteLine("[{0}:{1}] Starting connection ...", Configuration.IP, Configuration.Port);
                            m_socket.ConnectAsync(m_connect_args);
                        }
                        break;
                    }
                case GameConnectionState.Connected:
                    {
                        if ((m_socket.Poll(0, SelectMode.SelectRead) && m_socket.Available == 0) || !m_socket.Connected)
                        {
                            System.Console.WriteLine("[{0}:{1}] Disconnected.", Configuration.IP, Configuration.Port);

                            ChangeState(GameConnectionState.Disconnected);
                            m_connectionBackOffTimer = ConnectionFailureBackoff;
                        }
                        else
                        {
                            if (m_recieving_payload)
                            {
                                if (m_socket.Available >= 0)
                                {
                                    int toRead = Math.Min((int)(m_payload_size - m_payload_buffer_read), (int)m_socket.Available);
                                    m_socket.Receive(m_payload_buffer, m_payload_buffer_read, toRead, SocketFlags.None);
                                    m_payload_buffer_read += toRead;

                                    if (m_payload_buffer_read >= m_payload_size)
                                    {
                                        Recieve(m_payload_buffer);

                                        m_payload_buffer_read = 0;
                                        m_recieving_payload = false;
                                    }
                                }
                            }
                            else if (m_socket.Available > 4)
                            {
                                byte[] sizeBuffer = new byte[4];
                                m_socket.Receive(sizeBuffer);
                                m_payload_size = BitConverter.ToUInt32(sizeBuffer, 0);
                                m_payload_buffer = new byte[m_payload_size];
                                m_payload_buffer_read = 0;
                                m_recieving_payload = true;
                            }
                        }
                        break;
                    }
            }

            if (State == originalState)
            {
                m_stateTime += elapsed;
            }
        }

        void Recieve(byte[] Buffer)
        {            
            string Json = System.Text.Encoding.UTF8.GetString(Buffer);
           // System.Console.Write("Recv:\n" + Json + "\n\n");

            BasePacket Base = JsonConvert.DeserializeObject<BasePacket>(Json);
            Type PacketType = Type.GetType("GameMonitor.Connection." + Base.ClassName);
            BasePacket Full = JsonConvert.DeserializeObject(Json, PacketType) as BasePacket;
            if (Full != null)
            {
                System.Console.WriteLine("Recieved packet "+Full.ID+" (reply to "+Full.RequestID+").");

                Full.Process(this);

                // Any async events awaiting?
                foreach (QueuedPacketResponseEvent evt in m_queuedResponseEvents)
                {
                    if (evt.RequestID == Full.RequestID)
                    {
                        System.Console.WriteLine("Invoking async event for packet retrieval.");

                        evt.Event(this, Full);
                        m_queuedResponseEvents.Remove(evt);
                        break;
                    }
                }
            }
        }

        public void Send(BasePacket packet, PacketResponseEvent responseEvent = null)
        {
            string Json = JsonConvert.SerializeObject(packet);

            System.Console.WriteLine("Sent packet " + packet.ID + " (reply to " + packet.RequestID + ").");

            try
            {
                byte[] Buffer = System.Text.Encoding.UTF8.GetBytes(Json);
                byte[] LengthBuffer = BitConverter.GetBytes(Buffer.Length);
                m_socket.Send(LengthBuffer);
                m_socket.Send(Buffer);
            }
            catch (SocketException)
            {
                return;
            }

            if (responseEvent != null)
            {
                QueuedPacketResponseEvent evt = new QueuedPacketResponseEvent();
                evt.RequestID = packet.ID;
                evt.Event = responseEvent;
                m_queuedResponseEvents.Add(evt);
            }

            //System.Console.Write("Sent:\n" + Json + "\n\n");
        }

        void AsyncConnectCompleted(object sender, SocketAsyncEventArgs e)
        {
            if (e.SocketError == SocketError.Success)
            {
                System.Console.WriteLine("[{0}:{1}] Connection success.", Configuration.IP, Configuration.Port);
                ChangeState(GameConnectionState.Connected);
            }
            else
            {
                System.Console.WriteLine("[{0}:{1}] Connection failed.", Configuration.IP, Configuration.Port);

                ChangeState(GameConnectionState.Disconnected);
                m_connectionBackOffTimer = ConnectionFailureBackoff;
                m_recieving_payload = false;
                m_payload_buffer_read = 0;
            }
        }
    }
}
