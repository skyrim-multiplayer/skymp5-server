import React from 'react';

const D6 = (props: { className?: string; onClick?: () => void }) => {
  return (
    <div className="dice-wrapper">
      <svg
        width="48"
        onClick={props.onClick ? props.onClick : () => {}}
        className={`dice icon-d6 ${props.className}`}
        height="48"
        viewBox="0 0 48 48"
        fill="none"
        xmlns="http://www.w3.org/2000/svg"
      >
        <path
          d="M30.4499 21.15C30.4166 19.0167 29.6166 17.8334 28.0499 17.6L12.4499 15.4C11.4499 15.2667 10.7832 15.4667 10.4499 16C10.3832 16.1 10.3332 16.1834 10.2999 16.25C10.2999 16.2834 10.2832 16.3334 10.2499 16.4C10.2499 16.4667 10.2332 16.55 10.1999 16.65C10.1332 16.95 10.1332 17.3167 10.1999 17.75L10.7999 30.85C10.6666 33.05 11.3832 34.6167 12.9499 35.55L27.9999 38.6501C29.3332 38.1834 30.0499 37.1001 30.1499 35.4001L30.4499 21.15ZM23.6999 22.95C23.2666 22.75 22.7499 22.6167 22.1499 22.55C20.8499 22.35 19.8499 22.55 19.1499 23.15C18.4832 23.7834 18.1166 24.8834 18.0499 26.4501H18.1499C18.4499 26.0834 18.8332 25.8167 19.2999 25.65C19.7999 25.45 20.3666 25.3834 20.9999 25.45C22.1332 25.6167 23.0332 26.1 23.6999 26.9C24.3332 27.7 24.6499 28.6834 24.6499 29.85C24.6499 31.15 24.2832 32.1334 23.5499 32.8C22.8166 33.4334 21.7999 33.6667 20.4999 33.5C19.5999 33.3667 18.8166 33.0334 18.1499 32.5C17.5166 32.0334 17.0332 31.35 16.6999 30.4501C16.3332 29.5501 16.1499 28.5501 16.1499 27.4501C16.1499 22.5501 18.1332 20.3667 22.0999 20.9C22.7332 21 23.2666 21.1167 23.6999 21.25V22.95ZM18.1499 28.65C18.1499 29.45 18.3832 30.1667 18.8499 30.8C19.2832 31.4 19.8166 31.75 20.4499 31.85C21.1499 31.95 21.6999 31.8 22.0999 31.4C22.4666 31 22.6499 30.4 22.6499 29.6C22.6499 28.8667 22.4666 28.2667 22.0999 27.8C21.7666 27.3667 21.2332 27.1 20.4999 27C20.0666 26.9334 19.6666 26.9667 19.2999 27.1C18.9332 27.2334 18.6666 27.4501 18.4999 27.75C18.2666 27.9834 18.1499 28.2834 18.1499 28.65ZM32.2999 16.15L37.1499 12.45C37.3166 12.25 37.2166 11.9 36.8499 11.4L20.8499 9.80005C19.9832 9.86672 19.0499 10.1167 18.0499 10.55L11.6999 13.55C11.5999 14.3167 11.7999 14.7 12.2999 14.7L29.3999 17.15C30.3666 17.1834 31.3332 16.85 32.2999 16.15ZM38.7999 14.85C38.8332 14.3167 38.7999 13.9 38.6999 13.6C38.6999 13.5667 38.6999 13.5334 38.6999 13.5C38.4666 12.9 38.0332 12.6834 37.3999 12.85L32.6499 16.6C31.6499 17.4667 31.1166 18.6667 31.0499 20.2L30.7499 35.6501C30.7166 35.8501 30.6999 36.0334 30.6999 36.2001C30.7999 36.8667 30.9666 37.3 31.1999 37.5C31.3332 37.6 31.4999 37.6667 31.6999 37.7001C31.8999 37.7001 32.0832 37.6667 32.2499 37.6C32.3499 37.5 32.4499 37.4 32.5499 37.3L37.0499 32.05C37.4832 31.4167 37.7499 30.8167 37.8499 30.25C38.0832 29.6167 38.1832 28.9667 38.1499 28.3L38.7999 14.85Z"
          className="to-color"
        />
        <path
          d="M40.4502 11.95C40.4502 11.9166 40.4502 11.8833 40.4502 11.85C40.5502 10.5833 39.8335 9.78331 38.3002 9.44998L20.0502 7.59998C19.0169 7.63331 17.9335 7.91664 16.8002 8.44998L9.5002 11.85C8.53353 12.6833 8.06686 13.5666 8.1002 14.5L7.9002 15C7.9002 15.0333 7.9002 15.0833 7.9002 15.15C7.86686 15.2166 7.8502 15.3333 7.8502 15.5C7.81686 15.8333 7.81686 16.2333 7.8502 16.7L8.5502 31.75C8.38353 34.25 9.2002 36.0333 11.0002 37.1L28.2502 40.65C30.4169 41.15 32.0335 40.75 33.1002 39.45C33.0002 39.5166 32.9169 39.5666 32.8502 39.6C32.6835 39.6 32.5669 39.6 32.5002 39.6C32.6669 39.5666 32.8669 39.5166 33.1002 39.45C33.1335 39.4166 33.1669 39.3833 33.2002 39.35C33.2669 39.2833 33.3502 39.2166 33.4502 39.15L38.5502 33.05C39.2835 31.9833 39.7002 30.8833 39.8002 29.75C39.8335 29.4166 39.8502 29.1 39.8502 28.8L40.6002 13.4C40.6002 12.8 40.5502 12.3166 40.4502 11.95ZM38.7002 13.5C38.7002 13.5333 38.7002 13.5666 38.7002 13.6C38.8002 13.9 38.8335 14.3166 38.8002 14.85L38.1502 28.3C38.1835 28.9666 38.0835 29.6166 37.8502 30.25C37.7502 30.8166 37.4835 31.4166 37.0502 32.05L32.5502 37.3C32.4502 37.4 32.3502 37.5 32.2502 37.6C32.0835 37.6666 31.9002 37.7 31.7002 37.7C31.5002 37.6666 31.3335 37.6 31.2002 37.5C30.9669 37.3 30.8002 36.8666 30.7002 36.2C30.7002 36.0333 30.7169 35.85 30.7502 35.65L31.0502 20.2C31.1169 18.6666 31.6502 17.4666 32.6502 16.6L37.4002 12.85C38.0335 12.6833 38.4669 12.9 38.7002 13.5ZM37.1502 12.45L32.3002 16.15C31.3335 16.85 30.3669 17.1833 29.4002 17.15L12.3002 14.7C11.8002 14.7 11.6002 14.3166 11.7002 13.55L18.0502 10.55C19.0502 10.1166 19.9835 9.86664 20.8502 9.79998L36.8502 11.4C37.2169 11.9 37.3169 12.25 37.1502 12.45ZM28.0502 17.6C29.6169 17.8333 30.4169 19.0166 30.4502 21.15L30.1502 35.4C30.0502 37.1 29.3335 38.1833 28.0002 38.65L12.9502 35.55C11.3835 34.6166 10.6669 33.05 10.8002 30.85L10.2002 17.75C10.1335 17.3166 10.1335 16.95 10.2002 16.65C10.2335 16.55 10.2502 16.4666 10.2502 16.4C10.2835 16.3333 10.3002 16.2833 10.3002 16.25C10.3335 16.1833 10.3835 16.1 10.4502 16C10.7835 15.4666 11.4502 15.2666 12.4502 15.4L28.0502 17.6ZM22.1502 22.55C22.7502 22.6166 23.2669 22.75 23.7002 22.95V21.25C23.2669 21.1166 22.7335 21 22.1002 20.9C18.1335 20.3666 16.1502 22.55 16.1502 27.45C16.1502 28.55 16.3335 29.55 16.7002 30.45C17.0335 31.35 17.5169 32.0333 18.1502 32.5C18.8169 33.0333 19.6002 33.3666 20.5002 33.5C21.8002 33.6666 22.8169 33.4333 23.5502 32.8C24.2835 32.1333 24.6502 31.15 24.6502 29.85C24.6502 28.6833 24.3335 27.7 23.7002 26.9C23.0335 26.1 22.1335 25.6166 21.0002 25.45C20.3669 25.3833 19.8002 25.45 19.3002 25.65C18.8335 25.8166 18.4502 26.0833 18.1502 26.45H18.0502C18.1169 24.8833 18.4835 23.7833 19.1502 23.15C19.8502 22.55 20.8502 22.35 22.1502 22.55ZM18.5002 27.75C18.6669 27.45 18.9335 27.2333 19.3002 27.1C19.6669 26.9666 20.0669 26.9333 20.5002 27C21.2335 27.1 21.7669 27.3666 22.1002 27.8C22.4669 28.2666 22.6502 28.8666 22.6502 29.6C22.6502 30.4 22.4669 31 22.1002 31.4C21.7002 31.8 21.1502 31.95 20.4502 31.85C19.8169 31.75 19.2835 31.4 18.8502 30.8C18.3835 30.1666 18.1502 29.45 18.1502 28.65C18.1502 28.2833 18.2669 27.9833 18.5002 27.75Z"
          fill="black"
        />
      </svg>
    </div>
  );
};

export default D6;
